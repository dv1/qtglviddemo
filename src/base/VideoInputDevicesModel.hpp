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


#ifndef QTGLVIDDEMO_VIDEO_INPUT_DEVICES_MODEL_HPP
#define QTGLVIDDEMO_VIDEO_INPUT_DEVICES_MODEL_HPP

#include <map>
#include <QAbstractListModel>


namespace qtglviddemo
{


/**
 * Qt list data model containing a list of video input devices.
 *
 * The list contains entries with two roles: one for device node strings,
 * one for the user-readable name of the device.
 *
 * This list updates itself by listening to udev events.
 */
class VideoInputDevicesModel
	: public QAbstractListModel
{
	Q_OBJECT

public:
	typedef std::map < QString, QString > DeviceNodeNameMap;

	/**
	 * Constructor.
	 *
	 * Sets up and starts udev based device monitoring.
	 *
	 * @param p_parent Parent QObject
	 */
	explicit VideoInputDevicesModel(QObject *p_parent = nullptr);
	/**
	 * Destructor.
	 *
	 * Stops any ongoing udev based device monitoring.
	 */
	~VideoInputDevicesModel();

	enum Roles
	{
		DeviceNodeRole = Qt::UserRole + 1,
		DeviceNameRole
	};

	/**
	 * Retrieves all data roles for the given list entry.
	 *
	 * The returned value is a QVariantMap containing one entry for each
	 * of the data roles of the entry at the given row (= the item index).
	 */
	Q_INVOKABLE QVariantMap get(int p_row) const;

	void setDeviceNodeNameMap(DeviceNodeNameMap p_deviceNodeNameMap);
	DeviceNodeNameMap const & getDeviceNodeNameMap() const;

	// QAbstractListModel overloads
	virtual QVariant data(QModelIndex const &p_index, int p_role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(QModelIndex const &index) const override;
	virtual QVariant headerData(int p_section, Qt::Orientation p_orientation, int p_role = Qt::DisplayRole) const override;
	virtual QModelIndex parent(QModelIndex const &p_index) const override;
	virtual QHash < int, QByteArray > roleNames() const override;
	virtual int rowCount(QModelIndex const &p_parent = QModelIndex()) const override;


private slots:
	void handleUDevNotification();


private:
	void enumerateDevices();

	struct Priv;
	Priv *m_priv;
};


} // namespace qtglviddemo end


#endif
