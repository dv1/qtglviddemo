/**
 * Qt5 OpenGL video demo application
 * Copyright (C) 2018 Carlos Rafael Giani < dv AT pseudoterminal DOT org >
 *
 * qtglviddemo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <algorithm>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <vector>
#include <libudev.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <QByteArray>
#include <QHash>
#include <QVariantMap>
#include <QDebug>
#include <QLoggingCategory>
#include <QSocketNotifier>
#include "ScopeGuard.hpp"
#include "VideoInputDevicesModel.hpp"


Q_DECLARE_LOGGING_CATEGORY(lcQtGLVidDemo)


namespace qtglviddemo
{


struct DeviceEntry
{
	QString m_name, m_node;
	QString m_overrideName;
};


namespace
{


bool isV4L2CaptureDevice(char const *p_deviceNode)
{
	int fd = -1;
	auto cleanup = makeScopeGuard([&]() {
		if (fd >= 0)
			close(fd);
	});

	// Check if the given device node really is a character device.

	struct stat st;
	if (stat(p_deviceNode, &st) == -1)
	{
		qCCritical(lcQtGLVidDemo) << "Could not stat device" << p_deviceNode << ":" << std::strerror(errno);
		return false;
	}

	if (!S_ISCHR(st.st_mode))
	{
		qCCritical(lcQtGLVidDemo) << p_deviceNode << " is not a character device";
		return false;
	}

	// Open the device for V4L2 ioctls.
	fd = open(p_deviceNode, O_RDWR);
	if (fd < 0)
	{
		qCCritical(lcQtGLVidDemo) << "Could not open device" << p_deviceNode << ":" << std::strerror(errno);
		return false;
	}

	// Get the device capabilities.
	struct v4l2_capability vidcaps;
	if (ioctl(fd, VIDIOC_QUERYCAP, &vidcaps) < 0)
	{
		qCCritical(lcQtGLVidDemo) << "Could not get Video4Linux2 capabilities from device" << p_deviceNode << ":" << std::strerror(errno);
		return false;
	}

	// Pick the right capabilities field according to the presence or absence
	// of V4L2_CAP_DEVICE_CAPS.
	// See https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/vidioc-querycap.html
	// for details.
	std::uint32_t deviceCaps;
	if (vidcaps.capabilities & V4L2_CAP_DEVICE_CAPS)
		deviceCaps = vidcaps.device_caps;
	else
		deviceCaps = vidcaps.capabilities;

	// Check if the device capabilities contain capture bits
	return (deviceCaps & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE)) != 0;
}


void checkModelName(DeviceEntry &p_devEntry, VideoInputDevicesModel::DeviceNodeNameMap const &p_deviceNodeNameMap)
{
	// If this device node is listed in the device node name map,
	// set the m_overrideName field.
	auto iter = p_deviceNodeNameMap.find(p_devEntry.m_node);
	if (iter != p_deviceNodeNameMap.end())
	{
		p_devEntry.m_overrideName = iter->second;
		return;
	}

	// If the name is empty, use the node to produce a default name.
	if (p_devEntry.m_name.isEmpty())
		p_devEntry.m_name = QString("Unnamed device at ") + p_devEntry.m_node;
}


} // unnamed namespace end


struct VideoInputDevicesModel::Priv
{
	typedef std::vector < DeviceEntry > DeviceList;
	DeviceList m_deviceList;

	struct udev *m_udevContext;
	struct udev_monitor *m_udevMonitor;
	int m_udevMonitorFileDescriptor;
	QSocketNotifier *m_udevSocketNotifier;

	DeviceNodeNameMap m_deviceNodeNameMap;

	Priv()
		: m_udevContext(nullptr)
		, m_udevMonitor(nullptr)
		, m_udevMonitorFileDescriptor(-1)
		, m_udevSocketNotifier(nullptr)
	{
		auto guard = makeScopeGuard([&]() {
			delete m_udevSocketNotifier;
			if (m_udevMonitor != nullptr)
				udev_monitor_unref(m_udevMonitor);
			if (m_udevContext != nullptr)
				udev_unref(m_udevContext);
		});

		// Get the udev context and monitor, and set up the monitor
		// to listen for Video4Linux2 device events.

		m_udevContext = udev_new();
		if (m_udevContext == nullptr)
		{
			qCCritical(lcQtGLVidDemo) << "Could not create udev context";
			return;
		}

		m_udevMonitor = udev_monitor_new_from_netlink(m_udevContext, "udev");
		if (m_udevMonitor == nullptr)
		{
			qCCritical(lcQtGLVidDemo) << "Could not create udev monitor";
			return;
		}

		int err = udev_monitor_filter_add_match_subsystem_devtype(m_udevMonitor, "video4linux", 0);
		if (err != 0)
		{
			qCCritical(lcQtGLVidDemo) << "Could not add Video4Linux2 device type match to udev monitor:" << std::strerror(-err);
			return;
		}

		// Get the file descriptor used by the udev monitor.
		m_udevMonitorFileDescriptor = udev_monitor_get_fd(m_udevMonitor);
		if (m_udevMonitorFileDescriptor < 0)
		{
			qCCritical(lcQtGLVidDemo) << "Could not get file descriptor from udev monitor:" << std::strerror(-m_udevMonitorFileDescriptor);
			return;
		}

		// Create a socket notifier that shall listen to the udev monitor's
		// file descriptor. This way, we can listed for udev events.
		m_udevSocketNotifier = new QSocketNotifier(m_udevMonitorFileDescriptor, QSocketNotifier::Read, nullptr);

		guard.dismiss();
	}

	~Priv()
	{
		delete m_udevSocketNotifier;

		if (m_udevMonitor != nullptr)
			udev_monitor_unref(m_udevMonitor);

		if (m_udevContext != nullptr)
			udev_unref(m_udevContext);
	}

	DeviceList::iterator findDevice(QString const &p_deviceNode)
	{
		return std::find_if(m_deviceList.begin(), m_deviceList.end(), [&](DeviceEntry const &p_entry) -> bool {
			return p_entry.m_node == p_deviceNode;
		});
	}

	DeviceList::const_iterator findDevice(QString const &p_deviceNode) const
	{
		return std::find_if(m_deviceList.begin(), m_deviceList.end(), [&](DeviceEntry const &p_entry) -> bool {
			return p_entry.m_node == p_deviceNode;
		});
	}

	bool hasDevice(QString const &p_deviceNode) const
	{
		return findDevice(p_deviceNode) != m_deviceList.end();
	}
};


VideoInputDevicesModel::VideoInputDevicesModel(QObject *p_parent)
	: QAbstractListModel(p_parent)
	, m_priv(nullptr)
{
	m_priv = new Priv;
	auto guard = makeScopeGuard([&]() { delete m_priv; });

	// Exit here if setting up udev objects failed.
	if (m_priv->m_udevContext == nullptr)
		return;

	connect(m_priv->m_udevSocketNotifier, &QSocketNotifier::activated, this, &VideoInputDevicesModel::handleUDevNotification);

	// Start listening for udev events.
	int err = udev_monitor_enable_receiving(m_priv->m_udevMonitor);
	if (err != 0)
	{
		qCCritical(lcQtGLVidDemo) << "Could not enable udev monitor reception:" << std::strerror(-err);
		return;
	}

	// Enumerate any already present video input devices. Do this
	// _after_ starting the udev monitor to make sure we can't miss
	// any devices that might be connected while this constructor
	// is executed.
	enumerateDevices();

	guard.dismiss();

}


VideoInputDevicesModel::~VideoInputDevicesModel()
{
	delete m_priv;
}


QVariantMap VideoInputDevicesModel::get(int p_row) const
{
	// This code was adapted from: https://stackoverflow.com/a/25652320/560774

	QHash < int, QByteArray > names = roleNames();
	QHashIterator < int, QByteArray > iter(names);
	QModelIndex idx = index(p_row, 0);
	QVariantMap res;
	while (iter.hasNext())
	{
		iter.next();
		QVariant data = idx.data(iter.key());
		res[iter.value()] = data;
	}
	return res;
}


void VideoInputDevicesModel::setDeviceNodeNameMap(DeviceNodeNameMap p_deviceNodeNameMap)
{
	m_priv->m_deviceNodeNameMap = std::move(p_deviceNodeNameMap);

	for (auto & devEntry : m_priv->m_deviceList)
	{
		auto iter = m_priv->m_deviceNodeNameMap.find(devEntry.m_node);
		if (iter != m_priv->m_deviceNodeNameMap.end())
			devEntry.m_overrideName = iter->second;
		else
			devEntry.m_overrideName = "";
	}
}


VideoInputDevicesModel::DeviceNodeNameMap const & VideoInputDevicesModel::getDeviceNodeNameMap() const
{
	return m_priv->m_deviceNodeNameMap;
}


QVariant VideoInputDevicesModel::data(QModelIndex const &p_index, int p_role) const
{
	int idx = p_index.row();
	if ((idx < 0) || (idx >= int(m_priv->m_deviceList.size())))
		return QVariant();

	DeviceEntry const & devEntry = m_priv->m_deviceList[idx];

	switch (p_role)
	{
		case DeviceNodeRole: return QVariant::fromValue(devEntry.m_node);
		case DeviceNameRole: return QVariant::fromValue(devEntry.m_overrideName.isEmpty() ? devEntry.m_name : devEntry.m_overrideName);
		default: return QVariant();
	}
}


Qt::ItemFlags VideoInputDevicesModel::flags(QModelIndex const &) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant VideoInputDevicesModel::headerData(int, Qt::Orientation, int p_role) const
{
	if (p_role != Qt::DisplayRole)
		return QVariant();

	return "Name";
}


QModelIndex VideoInputDevicesModel::parent(QModelIndex const &) const
{
	return QModelIndex();
}


QHash < int, QByteArray > VideoInputDevicesModel::roleNames() const
{
	QHash < int, QByteArray > names;
	names[DeviceNodeRole] = "deviceNode";
	names[DeviceNameRole] = "deviceName";
	return names;
}


int VideoInputDevicesModel::rowCount(QModelIndex const &) const
{
	return m_priv->m_deviceList.size();
}


void VideoInputDevicesModel::handleUDevNotification()
{
	struct udev_device *udevice = nullptr;
	DeviceEntry devEntry;

	auto cleanup = makeScopeGuard([&]() {
		if (udevice != nullptr)
			udev_device_unref(udevice);
	});

	// Retrieve udev device associated with the monitor.
	udevice = udev_monitor_receive_device(m_priv->m_udevMonitor);
	if (udevice == nullptr)
		return;

	// Get the action that triggered this udev notification.
	char const *actionCStr = udev_device_get_action(udevice);
	if (actionCStr == nullptr)
		return;

	// Get the device node path associated with the udev device that
	// triggered the udev notification.
	char const *devNodeCStr = udev_device_get_devnode(udevice);
	if (devNodeCStr == nullptr)
		return;
	devEntry.m_node = QString::fromUtf8(devNodeCStr);

	// Handle the event according to the given action.

	if ((std::strcmp(actionCStr, "add") == 0) && !(m_priv->hasDevice(devEntry.m_node)))
	{
		// Add the new device to the list (unless it is there already).

		// If this is not a Video4Linux2 capture device, we ignore
		// the notification.
		if (!isV4L2CaptureDevice(devNodeCStr))
			return;

		beginInsertRows(QModelIndex(), m_priv->m_deviceList.size(), m_priv->m_deviceList.size());

		// Retrieve the added device's model name.
		char const *devNameCStr = udev_device_get_property_value(udevice, "ID_MODEL");
		devEntry.m_name = QString::fromUtf8(devNameCStr);
		checkModelName(devEntry, m_priv->m_deviceNodeNameMap);
		m_priv->m_deviceList.emplace_back(devEntry);

		endInsertRows();

		qCDebug(lcQtGLVidDemo) << "Added V4L2 device at" << devEntry.m_node << "model" << devEntry.m_name;
	}
	else if (std::strcmp(actionCStr, "remove") == 0)
	{
		// Remove the device from the list (unless it isn't there).

		auto iter = m_priv->findDevice(devEntry.m_node);
		if (iter != m_priv->m_deviceList.end())
		{
			int idx = (iter - m_priv->m_deviceList.begin());

			beginRemoveRows(QModelIndex(), idx, idx);
			m_priv->m_deviceList.erase(iter);
			endRemoveRows();

			qCDebug(lcQtGLVidDemo) << "Removed V4L2 device at" << devEntry.m_node;
		}
	}
}


void VideoInputDevicesModel::enumerateDevices()
{
	// Create udev device enumerator and limit its scope to
	// Video4Linux2 devices.
	udev_enumerate *uenumerate = udev_enumerate_new(m_priv->m_udevContext);
	udev_enumerate_add_match_subsystem(uenumerate, "video4linux");

	auto cleanup = makeScopeGuard([&]() {
		if (uenumerate != nullptr)
			udev_enumerate_unref(uenumerate);
	});

	// Enumerate all Video4Linux2 devices and fill the
	// enumerator's match list.
	udev_enumerate_scan_devices(uenumerate);

	udev_list_entry *entry;
	udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(uenumerate))
	{
		DeviceEntry devEntry;

		// Retrieve enumerated udev device.
		const char *syspath = udev_list_entry_get_name(entry);
		udev_device *udevice = udev_device_new_from_syspath(m_priv->m_udevContext, syspath);
		auto cleanupDevice = makeScopeGuard([&]() { udev_device_unref(udevice); });

		// Retrieve the enumerated device's node path.
		char const *devNodeCStr = udev_device_get_devnode(udevice);

		// If this is not a Video4Linux2 capture device, we
		// skip the device in the enumeration.
		if (!isV4L2CaptureDevice(devNodeCStr))
			continue;

		devEntry.m_node = QString::fromUtf8(devNodeCStr);

		// Retrieve the enumerated device's model name.
		char const *devNameCStr = udev_device_get_property_value(udevice, "ID_MODEL");
		devEntry.m_name = QString::fromUtf8(devNameCStr);

		// Check if the device model name needs to be fixed.
		checkModelName(devEntry, m_priv->m_deviceNodeNameMap);

		qCDebug(lcQtGLVidDemo) << "Found V4L2 device at" << devEntry.m_node << "model" << devEntry.m_name;

		// Finally, add the new entry for the enumerated device.
		m_priv->m_deviceList.emplace_back(devEntry);
	}
}


} // namespace qtglviddemo end
