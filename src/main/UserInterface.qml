import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0
import qtglviddemo 1.0


Window {
	Material.theme: Material.Dark
	id: window
	visible: true
	width: 1280
	height: 720
	color: "black"
	property var itemWidth: Math.floor(Math.min(window.width, window.height) * 0.8)
	property var itemHeight: itemWidth


	////////////////////
	// Misc functions //
	////////////////////

	// Pad a number with leading zeroes if the number has
	// fewer digits than indicated by the size argument
	function pad(num, size) {
		var s = num+"";
		while (s.length < size) s = "0" + s;
		return s;
	}

	// Convert an amount of milliseconds into a HH:MM:SS string
	function getMsecsAsTime(ms) {
		if (ms >= 0) {
			var hours = Math.floor(ms / 3600000);
			var minutes = Math.floor(ms / 60000) % 60;
			var seconds = Math.floor(ms / 1000) % 60;

			return pad(hours, 2) + ":" + pad(minutes, 2) + ":" + pad(seconds, 2);
		} else
			return "-";
	}


	Component.onCompleted: {
		if (splashscreenUrl == "")
			nonSplashscreenStartup.start();
		else
			splashscreenStartup.start();
	}

	SequentialAnimation {
		id: splashscreenStartup
		running: false

		ParallelAnimation {
			NumberAnimation { target: splashscreen; property: "opacity"; easing.type: Easing.Linear; from: 0; to: 1; duration: 2000 }
			NumberAnimation { target: background; property: "opacity"; easing.type: Easing.Linear; from: 0; to: 1; duration: 2000 }
		}
		ParallelAnimation {
			// Set the "to" value to 1 if keepSplashscreen is set to true
			// to make the splashscreen remain visible in the background
			NumberAnimation { target: splashscreen; property: "opacity"; easing.type: Easing.Linear; from: 1; to: (keepSplashscreen ? 1 : 0); duration: 2000 }
			NumberAnimation { target: mainScreenSection; property: "opacity"; easing.type: Easing.InCubic; from: 0; to: 1; duration: 2000 }
		}
	}
	SequentialAnimation {
		id: nonSplashscreenStartup
		running: false
		ParallelAnimation {
			NumberAnimation { target: background; property: "opacity"; easing.type: Easing.Linear; from: 0; to: 1; duration: 1000 }
			NumberAnimation { target: mainScreenSection; property: "opacity"; easing.type: Easing.InCubic; from: 0; to: 1; duration: 1000 }
		}
	}



	Item {
		id: splashscreen
		opacity: 0
		z: 1
		anchors.fill: parent
		Image {
			anchors.fill: parent
			anchors.margins: 20
			source: splashscreenUrl
			fillMode: Image.PreserveAspectFit
		}
		MouseArea {
			anchors.fill: parent
			enabled: parent.opacity != 0
		}
	}


	/////////////////////////
	// Video item delegate //
	/////////////////////////

	Component {
		id: videoObjectDelegate
		VideoObject {
			width: window.itemWidth * PathView.itemScale * PathView.itemScale * objScale
			height: window.itemHeight * PathView.itemScale * PathView.itemScale * objScale

			// Bind item properties to the data model.
			// obj* properties are the item data roles from the video
			// object data model. This data model is connected to the
			// path view, which in turn uses VideoObject as its delegate.

			meshType: objMeshType
			rotation: objRotation
			// The item's opacity is affected by both the opacity value
			// from the data model and the opacity value from the path view,
			// because the items that are placed further in the back are
			// drawn more translucent.
			opacity: objOpacity * PathView.itemOpacity
			cropRectangle: objCropRectangle
			textureRotation: objTextureRotation

			// Set the depth value to the Z value from the path view.
			// This makes sure that the current item is the one at
			// the front, and that others are placed behind it.
			z: PathView.itemZ

			// FBO contents use pixel coordinate (0,0) as the top left
			// corner, while OpenGL rendering uses (0,0) as the
			// bottom left corner. Mirror the FBO vertically to reconcile
			// these two.
			mirrorVertically: true

			// Resize the FBO when the item is resized, otherwise the FBO
			// pixels and the screen pixels aren't the same size.
			textureFollowsItemSize: true

			// Create custom properties to be able to modify the data model
			// properties from the outside. The rest of the QML code here
			// modifies the data model properties using the obj*Value
			// properties.
			property var meshTypeValue : objMeshType
			property var subtitleSourceValue : objSubtitleSource
			property var scaleValue : objScale
			property var opacityValue : objOpacity
			property var cropRectangleValue : objCropRectangle
			property var textureRotationValue : objTextureRotation
			onMeshTypeValueChanged: objMeshType = meshTypeValue
			onSubtitleSourceValueChanged: objSubtitleSource = subtitleSourceValue
			onScaleValueChanged: objScale = scaleValue
			onOpacityValueChanged: objOpacity = opacityValue
			onCropRectangleChanged: objCropRectangle = cropRectangleValue
			onTextureRotationValueChanged: objTextureRotation = textureRotationValue

			onCanStartPlayback: {
				// If playback ends, restart it.
				player.onEndOfStream.connect(function() { player.play(); });

				// Autostart playback.
				player.url = objUrl;
				player.play();
			}

			// Create mouse area so users can click on 3D objects to
			// select them (= making them the current item).
			MouseArea {
				anchors.fill: parent
				enabled: itemView.currentIndex != index
				onClicked: {
					itemView.currentIndex = index
				}
			}
		}
	}


	////////////////
	// Background //
	////////////////

	Rectangle {
		id: background
		anchors.fill: parent
		opacity: 0
		z: 0
		gradient: Gradient {
			GradientStop { position: 0.0; color: Qt.rgba(0.25, 0.25, 0.25, 1.00) }
			GradientStop { position: 1.0; color: Qt.rgba(0.36, 0.54, 0.60, 1.00) }
		}
	}


	/////////////////////////
	// Main screen section //
	/////////////////////////

	Item {
		id: mainScreenSection
		anchors.fill: parent
		opacity: 0
		z: 2


		//////////////////////
		// Controls sidebar //
		//////////////////////

		Rectangle {
			id: controls
			color: "#88222222"
			anchors.left: parent.left
			anchors.top: parent.top
			anchors.bottom: parent.bottom
			anchors.leftMargin: 10
			anchors.topMargin: 10
			anchors.rightMargin: 10
			anchors.bottomMargin: playbar.height + 40
			width: parent.width * 0.3
			radius: 10
			z: 20

			state: "VISIBLE"
			states: [
				State {
					name: "VISIBLE"
					PropertyChanges { target: controls; anchors.leftMargin: 10 }
				},
				State {
					name: "HIDDEN"
					PropertyChanges { target: controls; anchors.leftMargin: -width }
				}
			]

			transitions: [
				Transition {
					PropertyAnimation {
						properties: "anchors.leftMargin"
						easing.type: Easing.InOutCubic
						duration: 800
					}
				}
			]

			// This dummy mousearea is here to block
			// any mouse events that might otherwise
			// reach items that are currently placed
			// below the sidebar
			MouseArea {
				anchors.fill: parent
			}

			// In here, there are controls for adjusting various video object
			// properties. To avoid binding data loops, onValueChanged handlers
			// check if the value really changed. If not, then the current item's
			// associated data model properties are not modified. If this were
			// not done, endless loops could occur, because then the data model
			// itself would signal that data got changed, then the code here would
			// get notified about this and set the properties etc.
			ColumnLayout {
				anchors.fill: parent
				anchors.margins: 10
				Layout.alignment: Qt.AlignTop | Qt.AlignLeft
				spacing: 5
				GridLayout {
					Layout.fillWidth: true
					Layout.fillHeight: false
					columns: 2
					Label { text: "Scale" }
					Slider {
						Layout.fillWidth: true
						from: 0
						to: 1
						value: itemView.currentItem ? itemView.currentItem.scaleValue : 1
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.scaleValue !== value)
									itemView.currentItem.scaleValue = value;
							}
						}
					}
					Label { text: "Opacity" }
					Slider {
						Layout.fillWidth: true
						from: 0
						to: 1
						value: itemView.currentItem ? itemView.currentItem.opacityValue : 1
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.opacityValue !== value)
									itemView.currentItem.opacityValue = value;
							}
						}
					}
					Label { text: "2D rotation" }
					Slider {
						Layout.fillWidth: true
						from: 0
						to: 359
						value: itemView.currentItem ? itemView.currentItem.textureRotationValue : 0
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.textureRotationValue !== value)
									itemView.currentItem.textureRotationValue = value;
							}
						}
					}
					Label { text: "Mesh type" }
					ComboBox {
						id: meshTypeControl
						Layout.fillWidth: true
						textRole: "text"
						model: ListModel {
							ListElement {
								text: "Cube"
								meshType: "cube"
							}
							ListElement {
								text: "Teapot"
								meshType: "teapot"
							}
							ListElement {
								text: "Quad"
								meshType: "quad"
							}
							ListElement {
								text: "Sphere"
								meshType: "sphere"
							}
							ListElement {
								text: "Torus"
								meshType: "torus"
							}
						}

						property var meshType: itemView.currentItem.meshTypeValue

						onActivated: {
							var desc = model.get(currentIndex);
							if (desc !== undefined)
								itemView.currentItem.meshTypeValue = desc.meshType;
						}

						onMeshTypeChanged: {
							var curItem = itemView.currentItem;

							var index = 0;
							for (var i = 0; i < model.count; i++) {
								var elem = model.get(i);
								if (elem.meshType == curItem.meshTypeValue) {
									index = i;
									break;
								}
							}
							if (currentIndex !== index)
								currentIndex = index;
						}
					}
					Label { text: "Subtitle source" }
					ComboBox {
						Layout.fillWidth: true
						textRole: "text"
						model: ListModel {
							ListElement {
								text: "Media source"
								subtitleSource: VideoObjectModel.MediaSubtitles
							}
							ListElement {
								text: "FIFO"
								subtitleSource: VideoObjectModel.FIFOSubtitles
							}
						}

						property var subtitleSource: itemView.currentItem.subtitleSourceValue

						onActivated: {
							var desc = model.get(currentIndex);
							if (desc !== undefined)
								itemView.currentItem.subtitleSourceValue = desc.subtitleSource;
						}

						onSubtitleSourceChanged: {
							var curItem = itemView.currentItem;

							var index = 0;
							for (var i = 0; i < model.count; i++) {
								var elem = model.get(i);
								if (elem.subtitleSource == curItem.subtitleSourceValue) {
									index = i;
									break;
								}
							}
							if (currentIndex !== index)
								currentIndex = index;
						}
					}
				}
				Label {
					Layout.fillWidth: true
					Layout.fillHeight: false
					horizontalAlignment: Text.AlignHCenter
					text: "Crop"
				}
				GridLayout {
					id: cropControls
					Layout.fillWidth: true
					Layout.fillHeight: false
					columns: 4

					// Crop X value
					Label { text: "x"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
					SpinBox {
						id: cropX
						editable: true
						Layout.fillWidth: true
						from: 0; to: 100
						value: itemView.currentItem ? itemView.currentItem.cropRectangleValue.x : 0
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.cropRectangleValue.x !== value)
									itemView.currentItem.cropRectangleValue.x = value;
							}
						}
					}

					// Crop Y value
					Label { text: "y"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
					SpinBox {
						id: cropY
						editable: true
						Layout.fillWidth: true
						from: 0; to: 100
						value: itemView.currentItem ? itemView.currentItem.cropRectangleValue.y : 0
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.cropRectangleValue.y !== value)
									itemView.currentItem.cropRectangleValue.y = value;
							}
						}
					}

					// Crop width
					Label { text: "width"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
					SpinBox {
						id: cropW
						editable: true
						Layout.fillWidth: true
						from: 0; to: 100
						value: itemView.currentItem ? itemView.currentItem.cropRectangleValue.width : 100
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.cropRectangleValue.width !== value)
									itemView.currentItem.cropRectangleValue.width = value;
							}
						}
					}

					// Crop height
					Label { text: "width"; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
					SpinBox {
						id: cropH
						editable: true
						Layout.fillWidth: true
						from: 0; to: 100
						value: itemView.currentItem ? itemView.currentItem.cropRectangleValue.height : 100
						onValueChanged: {
							if (itemView.currentItem) {
								if (itemView.currentItem.cropRectangleValue.height !== value)
									itemView.currentItem.cropRectangleValue.height = value;
							}
						}
					}
				}
				GroupBox {
					Layout.fillWidth: true
					title: "Add media from"
					RowLayout {
						anchors.fill: parent
						MessageDialog {
							id: msgErrorBox
							modality: Qt.WindowModal
							icon: StandardIcon.Critical
						}
						FileDialog {
							id: fileDialog
							title: "Select a media file to add"
							onAccepted: itemView.currentIndex = videoObjectModel.addFromURL(fileUrl)
						}
						Dialog {
							id: captureDeviceDialog
							title: "Select the capture device to use"
							modality: Qt.WindowModal
							standardButtons: StandardButton.Open | StandardButton.Cancel
							onAccepted: {
								var devNode = videoInputDevicesModel.get(devicesCombobox.currentIndex).deviceNode;
								itemView.currentIndex = videoObjectModel.addV4L2DeviceNode(devNode);
							}
							onRejected: {
							}

							RowLayout {
								anchors.fill: parent
								Label {
									Layout.fillWidth: false
									text: "Device"
								}
								ComboBox {
									id: devicesCombobox
									Layout.fillWidth: true
									model: videoInputDevicesModel
									textRole: "deviceName"
									Layout.preferredWidth: window.width * 0.7
								}
							}
						}
						Dialog {
							id: urlDialog
							title: "Input the URL of the media to add"
							modality: Qt.WindowModal
							standardButtons: StandardButton.Open | StandardButton.Cancel
							onAccepted: {
								var streamUrl = urlLine.text;
								var itemIndex = videoObjectModel.addFromURL(streamUrl);
								if (itemIndex < 0) {
									msgErrorBox.text = "URL " + streamUrl + " is invalid";
									msgErrorBox.open();
								}
								else
									itemView.currentIndex = itemIndex;
							}
							onRejected: {
								urlLine.text = "";
							}

							RowLayout {
								anchors.fill: parent
								Label {
									Layout.fillWidth: false
									text: "URL"
								}
								TextField {
									id: urlLine
									Layout.fillWidth: true
									onAccepted: urlDialog.click(StandardButton.Open)
								}
							}
						}
						Button {
							text: "Local file"
							onClicked: fileDialog.open()
						}
						Button {
							text: "Capture device"
							onClicked: captureDeviceDialog.open()
						}
						Button {
							text: "URL"
							onClicked: urlDialog.open()
						}
					}
				}
				Button {
					Layout.fillWidth: true
					text: "Remove current media"
					onClicked: videoObjectModel.remove(itemView.currentIndex)
				}
				Item {
					Layout.fillWidth: true
					Layout.fillHeight: true
				}
			}
		}


		///////////////////////////
		// Main view and playbar //
		///////////////////////////

		ColumnLayout {
			anchors.fill: parent
			anchors.margins: 10

			// Main view, with a text overlay for the subtitles
			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true
				PathView {
					id: itemView
					anchors.fill: parent
					model: videoObjectModel
					delegate: videoObjectDelegate
					clip: true
					// Set the cache item cound to the model count to prevent
					// VideoObject instances from getting discarded. This would
					// otherwise happen if more than 3 video objects were visible
					// at the same time.
					cacheItemCount: model.count
					pathItemCount: 3
					preferredHighlightBegin: 0.5
					preferredHighlightEnd: 0.5
					snapMode: PathView.SnapToItem
					// Disable default PathView user interactions, otherwise
					// the custom MouseArea code in the delegate would conflict
					// with this functionality.
					interactive: false

					// Define a simple path for 3 elements. This path also has
					// attributes for scaling and opacity, to make non-current
					// items look smaller and more translucent. Also, the
					// itemZ attribute makes sure that the current item is
					// at the front, and the other ones are behind it.
					// In case there are only 2 elements, the attributes are
					// different, since the path view arranges 2 items
					// differently.
					path: Path {
						startX: itemView.width / 2 - window.itemWidth * ((itemView.model.count < 3) ? 0.65 : 1.0)
						startY: window.itemHeight / 2
						PathAttribute { name: "itemZ"; value: 0.0 }
						PathAttribute { name: "itemOpacity"; value: 0.2 }
						PathAttribute { name: "itemScale"; value: (itemView.model.count < 3) ? 0.55 : 0.3 }
						PathLine {
							x: itemView.width / 2
							y: itemView.height / 2
						}
						PathAttribute { name: "itemZ"; value: 0.9 }
						PathAttribute { name: "itemOpacity"; value: 1.0 }
						PathAttribute { name: "itemScale"; value: 1.0 }
						PathLine {
							x: itemView.width / 2 + window.itemWidth * ((itemView.model.count < 3) ? 0.65 : 1.0)
							y: window.itemHeight / 2
						}
					}
				}

				Text {
					id: subtitle
					color: "white"
					style: Text.Outline
					styleColor: "black"
					text: playerConnections.playbackSubtitle
					textFormat: Text.StyledText
					font.family: "Dosis"
					font.pointSize: 20
					anchors.bottom: parent.bottom
					anchors.top: parent.top
					anchors.right: parent.right
					anchors.left: parent.left
					anchors.bottomMargin: parent.height * 0.1
					anchors.topMargin: parent.height * 0.1
					anchors.rightMargin: parent.width * 0.1
					anchors.leftMargin: parent.width * 0.1
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignBottom
					wrapMode: Text.WordWrap
				}
			}

			// Playbar
			Rectangle {
				color: "#88222222"
				Layout.fillWidth: true
				Layout.fillHeight: false
				radius: 10
				height: 60
				z: 20

				RowLayout {
					id: playbar
					anchors.fill: parent
					anchors.margins: 10

					Button {
						Layout.fillWidth: false
						Layout.fillHeight: true
						text: "Toggle controls"
						onClicked: { controls.state = (controls.state == "VISIBLE") ? "HIDDEN" : "VISIBLE"; }
					}
					Button {
						Layout.fillWidth: false
						Layout.fillHeight: true
						text: "Previous"
						onClicked: itemView.decrementCurrentIndex()
					}
					Button {
						Layout.fillWidth: false
						Layout.fillHeight: true
						text: "Next"
						onClicked: itemView.incrementCurrentIndex()
					}
					Button {
						Layout.fillWidth: false
						Layout.fillHeight: true
						text: {
							switch (playerConnections.playbackState) {
								case GStreamerPlayer.Paused: return "Resume";
								case GStreamerPlayer.Playing: return "Pause";
								case GStreamerPlayer.Idle: return "Play";
								default: return "...";
							}
						}
						enabled: (playerConnections.playbackState == GStreamerPlayer.Paused) || (playerConnections.playbackState == GStreamerPlayer.Playing);
						onClicked: {
							if (itemView.currentItem != null) {
								var curState = itemView.currentItem.player.state;
								if ((curState === GStreamerPlayer.Playing) || (curState === GStreamerPlayer.Paused))
								{
									if (curState === GStreamerPlayer.Playing)
										itemView.currentItem.player.pause();
									else
										itemView.currentItem.player.play();
								}
							}
						}
					}
					Slider {
						id: positionSlider
						Layout.fillWidth: true
						Layout.fillHeight: true
						from: 0
						to: (playerConnections.playbackDuration >= 0) ? playerConnections.playbackDuration : 1
						enabled: playerConnections.playbackIsSeekable

						// This property is used to avoid endless loops between
						// onValueChanged and the onPlaybackSubtitleChanged slot.
						property var blockOnPositionChanged: false

						onValueChanged: {
							if (!blockOnPositionChanged && (itemView.currentItem != null))
								itemView.currentItem.player.seek(value);
						}
						Component.onCompleted: {
							playerConnections.onPlaybackPositionChanged.connect(function() {
								positionSlider.blockOnPositionChanged = true;
								positionSlider.value = playerConnections.playbackPosition;
								positionSlider.blockOnPositionChanged = false;
							});
						}
					}
					Label {
						id: positionLabel
						Layout.fillWidth: false
						Layout.fillHeight: true
						text: getMsecsAsTime(playerConnections.playbackPosition) + " / " + getMsecsAsTime(playerConnections.playbackDuration);
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter
					}
				}
			}
		}
	}


	/////////////////////////////////////////////////
	// Connections between data model and controls //
	/////////////////////////////////////////////////

	// This timer is used to erase the subtitles after a while.
	// Otherwise, they would stick around, and if no new subtitle
	// comes along, the current one would be shown indefinitely.
	// The interval value is updated by whoever starts the timer.
	Timer {
		id: subtitleTimer
		interval: 2000
		running: false
		repeat: false
		onTriggered: {
			if (playerConnections.playbackSubtitle !== "")
				playerConnections.playbackSubtitle = "";
		}
	}

	// The connections here are a way to decouple certain properties and signals of
	// the path view's current item from the rest of the code. By using this
	// decoupling, that other code does not have to worry about connecting/disconnecting
	// slots when the current item in the path view changes; it is done here instead.
	// The on* signals correspond to the signals of the same name in the current item.
	// This current item is set as the target. If there is no current item, target is
	// set to null.
	// This way, for example the playback position slide above merely has to connect
	// to the playbackPosition property here, and does not have to concern itself with
	// whether or not the current item changed.
	Connections {
		id: playerConnections
		target: null

		property var playbackPosition: 0
		property var playbackDuration: 1
		property var playbackState: GStreamerPlayer.Stopped
		property var playbackIsSeekable: false
		property var playbackSubtitle: ""

		onPositionUpdated: playbackPosition = newPosition;
		onDurationChanged: playbackDuration = newDuration;
		onStateChanged: playbackState = itemView.currentItem.player.state;
		onIsSeekableChanged: playbackIsSeekable = itemView.currentItem.player.isSeekable;
		onSubtitleChanged: {
			if (itemView.currentItem.subtitleSourceValue == VideoObjectModel.MediaSubtitles)
				playbackSubtitle = itemView.currentItem.player.subtitle;
		}

		function updateConnections() {
			var curItem = itemView.currentItem;
			if (curItem == null) {
				playerConnections.playbackPosition = 0;
				playerConnections.playbackDuration = 1;
				playerConnections.playbackState = GStreamerPlayer.Stopped;
				playerConnections.playbackIsSeekable = false;
				playerConnections.playbackSubtitle = "";
				playerConnections.target = null;
			} else {
				playerConnections.playbackPosition = curItem.player.position;
				playerConnections.playbackDuration = curItem.player.duration;
				playerConnections.playbackState = curItem.player.state;
				playerConnections.playbackIsSeekable = curItem.player.isSeekable;
				playerConnections.playbackSubtitle = curItem.player.subtitle;
				playerConnections.target = curItem.player;
			}
		}

		Component.onCompleted: {
			onPlaybackSubtitleChanged.connect(function() {
				// Start the subtitle timer to make sure the
				// subtitle disappears after a while. In case
				// the subtitle is very short, make it last
				// for 1 second, otherwise make the timer
				// timeout dependent on the string length.
				subtitleTimer.stop();
				subtitleTimer.interval = Math.max(playbackSubtitle.length * 80, 1000);
				subtitleTimer.start();
			});
			fifoWatch.onNewFifoLine.connect(function(line) {
				var curItem = itemView.currentItem;
				if ((curItem != null) && (curItem.subtitleSourceValue == VideoObjectModel.FIFOSubtitles))
					playerConnections.playbackSubtitle = line;
			});
			itemView.onCurrentItemChanged.connect(updateConnections);
			updateConnections();
		}
	}
}
