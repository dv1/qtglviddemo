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


#include <limits>
#include "VideoObjectModel.hpp"


namespace qtglviddemo
{


namespace
{


// Helper function to set data and to set a flag if the data really changed.
// This is necessary to avoid endless loops ("binding data loops" in QML jargon)
// which can happen in two-way data bindings. Example: a QtQuick 2 item modifies
// the opacity value. The data model's setData() function is called. Since the
// opacity value was overwritten, the dataChanged() signal is emitted. In the
// QML script, this causes the item's value to be modified. And this in turn
// causes the item to modifies the opacity value again ...
//
// By checking if the value actually changed, this can be prevented. If the
// "new" value is in fact the same as the current one, dataChanged() is not
// emitted. To determine if that signal needs to be emitted, p_valueChangedFlag
// is set to true only if the value actually changed.
template < typename T >
void setDataHelper(T & p_destValue, QVariant const &p_variant, bool &p_valueChangedFlag)
{
	T value = p_variant.value < T > ();
	if (p_destValue != value)
	{
		p_destValue = value;
		p_valueChangedFlag = true;
	}
}


} // unnamed namespace end


VideoObjectModel::Description::Description()
	: m_meshType("cube")
	, m_scale(1.0f)
	, m_opacity(1.0f)
	, m_cropRectangle(0, 0, 100, 100)
	, m_textureRotation(0)
	, m_subtitleSource(SubtitleSource::MediaSubtitles)
{
}


VideoObjectModel::VideoObjectModel(QObject *p_parent)
	: QAbstractListModel(p_parent)
{
}


int VideoObjectModel::addFromURL(QUrl p_url)
{
	if (!p_url.isValid())
		return -1;

	// Construct default description and set its URL.
	Description newDesc;
	newDesc.m_url = std::move(p_url);
	// Add the new description.
	addDescription(std::move(newDesc));

	// The description is always appended, meaning that its index is always
	// the new number of descriptions, minus one.
	return m_descriptions.size() - 1;
}


int VideoObjectModel::addV4L2DeviceNode(QString const &p_deviceNode)
{
	// Produce URL that can be used with GStreamer to receive rames from
	// the camera. If gstreamer-imx is used, we must use the imxv4l2://
	// URL scheme to instruct the GStreamer URI handler subsystem to
	// choose imxv4l2videosrc. This is essential, because v4l2src does
	// not work with mxc_v4l2 devices, and imxv4l2videosrc produces
	// frames that are stored in DMA buffers, thereby allowing for
	// zerocopy-enabled video rendering.
	QUrl url(
		QString(
#ifdef USE_IMX_V4L2
			"imxv4l2://"
#else
			"v4l2://"
#endif
		) + p_deviceNode
	);

	return addFromURL(std::move(url));
}


void VideoObjectModel::remove(int const p_index)
{
	removeDescription(p_index);
}


void VideoObjectModel::addDescription(Description p_description)
{
	// Let the base class know that we are adding a new entry (= a new row).
	beginInsertRows(QModelIndex(), m_descriptions.size(), m_descriptions.size());

	// Perform the actuall add operation.
	m_descriptions.emplace_back(p_description);

	// We are done modifying the model.
	endInsertRows();

	// Notify listeners that the count property changed (since there's now one
	// more description in the list).
	emit countChanged();
}


VideoObjectModel::Description const & VideoObjectModel::getDescription(std::size_t const p_index) const
{
	return m_descriptions[p_index];
}


std::size_t VideoObjectModel::getNumDescriptions() const
{
	return m_descriptions.size();
}


void VideoObjectModel::removeDescription(std::size_t const p_index)
{
	// Do nothing if the index is invalid.
	if (p_index >= std::size_t(m_descriptions.size()))
		return;

	// Let the base class know that we are remove an entry (= a row).
	beginRemoveRows(QModelIndex(), p_index, p_index);

	// Perform the actual remove operation.
	m_descriptions.erase(m_descriptions.begin() + p_index);

	// We are done modifying the model.
	endRemoveRows();

	// Notify listeners that the count property changed (since there's now one
	// less description in the list).
	emit countChanged();
}


QVariant VideoObjectModel::data(QModelIndex const &p_index, int p_role) const
{
	int idx = p_index.row();

	// If the index is invalid, return an empty value.
	if ((idx < 0) || (idx >= int(m_descriptions.size())))
		return QVariant();

	Description const & desc = m_descriptions[idx];

	switch (p_role)
	{
		case UrlRole:             return QVariant::fromValue(desc.m_url);
		case MeshTypeRole:        return QVariant::fromValue(desc.m_meshType);
		case ScaleRole:           return QVariant::fromValue(desc.m_scale);
		case RotationRole:        return QVariant::fromValue(desc.m_rotation);
		case OpacityRole:         return QVariant::fromValue(desc.m_opacity);
		case CropRectangleRole:   return QVariant::fromValue(desc.m_cropRectangle);
		case TextureRotationRole: return QVariant::fromValue(desc.m_textureRotation);
		case SubtitleSourceRole:  return QVariant::fromValue(desc.m_subtitleSource);
		default: return QVariant();
	}
}


bool VideoObjectModel::setData(QModelIndex const &p_index, const QVariant &p_value, int p_role)
{
	int idx = p_index.row();

	// If the index is invalid, return an empty value.
	if ((idx < 0) || (idx >= int(m_descriptions.size())))
		return false;

	Description & desc = m_descriptions[idx];
	bool valueGotChanged = false;

#define ROLE_VALUE_TO_DESC(VALNAME, VALTYPE) \
	setDataHelper < VALTYPE > (desc.m_ ##VALNAME, p_value, valueGotChanged)

	switch (p_role)
	{
		case UrlRole:             ROLE_VALUE_TO_DESC(url,             QUrl);           break;
		case MeshTypeRole:        ROLE_VALUE_TO_DESC(meshType,        QString);        break;
		case ScaleRole:           ROLE_VALUE_TO_DESC(scale,           float);          break;
		case RotationRole:        ROLE_VALUE_TO_DESC(rotation,        QQuaternion);    break;
		case OpacityRole:         ROLE_VALUE_TO_DESC(opacity,         float);          break;
		case CropRectangleRole:   ROLE_VALUE_TO_DESC(cropRectangle,   QRect);          break;
		case TextureRotationRole: ROLE_VALUE_TO_DESC(textureRotation, int);            break;
		case SubtitleSourceRole:  ROLE_VALUE_TO_DESC(subtitleSource,  SubtitleSource); break;
		default: return false;
	}

	if (valueGotChanged)
		emit dataChanged(p_index, p_index, { p_role });

	return true;
}


Qt::ItemFlags VideoObjectModel::flags(QModelIndex const &) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


QVariant VideoObjectModel::headerData(int, Qt::Orientation, int) const
{
	return QVariant();
}


QModelIndex VideoObjectModel::parent(QModelIndex const &) const
{
	return QModelIndex();
}


QHash < int, QByteArray > VideoObjectModel::roleNames() const
{
	// The role names have an "obj" prefix to make sure there is no name
	// collision with QtQuick property names ("opacity" is one example).
	// It also makes QML code a bit more readable, since if there's an
	// "obj" prefix, it is immediately clear that this is an item data
	// role value.
	QHash < int, QByteArray > names;
	names[UrlRole]             = "objUrl";
	names[MeshTypeRole]        = "objMeshType";
	names[ScaleRole]           = "objScale";
	names[RotationRole]        = "objRotation";
	names[OpacityRole]         = "objOpacity";
	names[CropRectangleRole]   = "objCropRectangle";
	names[TextureRotationRole] = "objTextureRotation";
	names[SubtitleSourceRole]  = "objSubtitleSource";
	return names;
}


int VideoObjectModel::rowCount(QModelIndex const &) const
{
	return m_descriptions.size();
}


int VideoObjectModel::getCount() const
{
	return getNumDescriptions();
}


} // namespace qtglviddemo end
