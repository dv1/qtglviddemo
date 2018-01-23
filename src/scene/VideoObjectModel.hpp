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


#ifndef QTGLVIDDEMO_VIDEO_OBJECT_MODEL_HPP
#define QTGLVIDDEMO_VIDEO_OBJECT_MODEL_HPP

#include <vector>
#include <QAbstractListModel>
#include <QQuaternion>
#include <QVector3D>
#include <QRectF>
#include <QUrl>


namespace qtglviddemo
{


/**
 * Qt list data model containing a list of video object descriptions.
 *
 * The entries in this data model only describe the video objects.
 * They do not contain any 3D mesh data or OpenGL resources. The
 * descriptions instead contain parameters such as 3D position,
 * video URL, mesh type string, etc.
 *
 * This model is designed to be usable in QML together with Qt Quick 2
 * views and VideoObjectItem as the delegate. That is, Qt Quick 2
 * views are connected to this model, and they use VideoObjectItem
 * to display the entries of this model.
 *
 * This model also allows for modifying the parameters of a description.
 * If for example the user rotates a video object on screen, the
 * rotation quaternion of the corresponding description in this model
 * is updated.
 *
 * Since this is derived from QAbstractListModel, entries are modeled
 * as "rows". So, 3 entries mean 3 rows for example. When QModelIndex
 * is used, only its row index is of any interest. (The list has only
 * one "column".) The terms "entry", "description", "row", "item" are
 * used interchangeably here. ("item" is not to be confused with
 * VideoObjectItem.)
 */
class VideoObjectModel
	: public QAbstractListModel
{
	Q_OBJECT
	/**
	 * How many descriptions are stored in the model.
	 *
	 * This is always equivalent to the return value of
	 * getNumDescriptions().
	 */
	Q_PROPERTY(int count READ getCount NOTIFY countChanged)

public:
	enum class SubtitleSource
	{
		/**
		 * Subtitles for the video object shall come from the FIFO.
		 * (See FifoWatch class for details.)
		 */
		FIFOSubtitles,
		/**
		 * Subtitles for the video object shall come from the
		 * video object's associated media player.
		 */
		MediaSubtitles,
		/**
		 * Subtitles for the video object shall come from system
		 * stat measurements (CPU usage, memory usage, framerate).
		 */
		SystemStatsSubtitles
	};
	Q_ENUM(SubtitleSource)

	/**
	 * Video object description.
	 */
	struct Description
	{
		/// URL of the media the video object shall play.
		QUrl m_url;

		/**
		 * String containing the type of the mesh the video object
		 * shall render. See the GLResources::getMesh() documenation
		 * for the list of valid mesh types.
		 */
		QString m_meshType;

		/// Scale factor the video object's transform shall use.
		float m_scale;
		/// Rotation quaternion the video object's transform shall use.
		QQuaternion m_rotation;
		/// Opacity that shall be used for rendering the VideoObjectItem.
		float m_opacity;
		/// Crop rectangle the video object's video material shall use.
		QRect m_cropRectangle;
		/// Texture rotation angle the video object's video material shall use.
		int m_textureRotation;

		/// Where the video object's subtitles shall come from.
		SubtitleSource m_subtitleSource;

		/**
		 * Constructor.
		 *
		 * Creates a description with mesh type "cube", scale factor
		 * 1, opacity 1, crop rectangle (0,0,100,100), a texture
		 * rotation angle of 0 degrees, and MediaSubtitles as the
		 * subtitle source.
		 */
		Description();
	};

	/**
	 * Model item data roles.
	 *
	 * A description has several parameters. These parameters are exposed
	 * to QML through item data roles. For each field in Description
	 * there is one user item data role defined here.
	 */
	enum DescriptionRoles
	{
		UrlRole = Qt::UserRole + 1,

		MeshTypeRole,

		ScaleRole,
		RotationRole,

		OpacityRole,
		CropRectangleRole,

		TextureRotationRole,

		SubtitleSourceRole
	};

	/**
	 * Constructor.
	 *
	 * @param p_parent Parent QObject
	 */
	explicit VideoObjectModel(QObject *p_parent = nullptr);

	/**
	 * Adds a description and sets its URL to the given URL.
	 *
	 * This produces a description with default values, except for the
	 * URL, which is set to p_url. Internally, it calls addDescription()
	 * for adding the new description.
	 *
	 * This returns an integer which can be used as list index in QML scripts.
	 *
	 * @param p_url URL to set in the new description.
	 */
	Q_INVOKABLE int addFromURL(QUrl p_url);
	/**
	 * Adds a description for a V4L2 device node.
	 *
	 * This is a convenience function to add descriptions with URLs
	 * that refer to video capture devices. Depending on the platform,
	 * these URLs may start with v4l2:// , or with something else.
	 *
	 * @param p_deviceNode Device node to use in the new description.
	 */
	Q_INVOKABLE int addV4L2DeviceNode(QString const &p_deviceNode);
	/**
	 * Removes the description at the given index.
	 *
	 * Internally, this calls removeDescription() for removing
	 * the description at the given index.
	 *
	 * @param p_index Index of the description to remove. Must be >= 0
	 *        and less than the value of the count property.
	 */
	Q_INVOKABLE void remove(int const p_index);

	/**
	 * Adds a description to the list.
	 *
	 * Since this effectively adds rows to the list (rows = entries),
	 * this will cause rowsAboutToBeInserted() and rowsInserted() signals
	 * to be emitted. Qt Quick 2 views use this to update themselves.
	 */
	void addDescription(Description p_description);
	/**
	 * Retrieves a const reference to the description at the given index.
	 *
	 * @param p_index Index of the description to remove. Must be >= 0
	 *        and less than what getNumDescriptions() returns.
	 */
	Description const & getDescription(std::size_t const p_index) const;
	/**
	 * Returns the number of description in this list model.
	 */
	std::size_t getNumDescriptions() const;
	/**
	 * Removes a description at the given index.
	 *
	 * Since this effectively adds rows to the list (rows = entries),
	 * this will cause rowsAboutToBeRemoved() and rowsRemoved() signals
	 * to be emitted. Qt Quick 2 views use this to update themselves.
	 *
	 * @param p_index Index of the description to remove. Must be >= 0
	 *        and less than what getNumDescriptions() returns.
	 */
	void removeDescription(std::size_t const p_index);

	// QAbstractItemModel and QAbstractListModel overrides.
	virtual QVariant data(QModelIndex const &p_index, int p_role = Qt::DisplayRole) const override;
	virtual bool setData(QModelIndex const &p_index, const QVariant &p_value, int p_role = Qt::EditRole) override;
	virtual Qt::ItemFlags flags(QModelIndex const &index) const override;
	virtual QVariant headerData(int p_section, Qt::Orientation p_orientation, int p_role = Qt::DisplayRole) const override;
	virtual QModelIndex parent(QModelIndex const &p_index) const override;
	virtual QHash < int, QByteArray > roleNames() const override;
	virtual int rowCount(QModelIndex const &p_parent = QModelIndex()) const override;


signals:
	void countChanged();


private:
	// Returns the current count of descriptions. This function exists only
	// because getNumDescriptions() can't be directly used in the Q_PROPERTY()
	// definition above (because of the type of the return value).
	int getCount() const;

	typedef std::vector < Description > Descriptions;
	Descriptions m_descriptions;
};


} // namespace qtglviddemo end


#endif
