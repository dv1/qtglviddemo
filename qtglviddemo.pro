PKGCONFIG += gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 gstreamer-app-1.0 gstreamer-player-1.0 libudev
CONFIG += qt c++11 link_pkgconfig moc
QT += core qml quick quickcontrols2 widgets


TARGET = qtglviddemo

SOURCES += \
	src/base/SystemStats.cpp \
	src/base/Utility.cpp \
	src/base/FifoWatch.cpp \
	src/base/VideoInputDevicesModel.cpp \
	src/mesh/QuadMesh.cpp \
	src/mesh/CubeMesh.cpp \
	src/mesh/TeapotMesh.cpp \
	src/mesh/SphereMesh.cpp \
	src/mesh/TorusMesh.cpp \
	src/mesh/Mesh.cpp \
	src/scene/GLResources.cpp \
	src/scene/Transform.cpp \
	src/scene/Camera.cpp \
	src/scene/Arcball.cpp \
	src/scene/VideoObjectModel.cpp \
	src/scene/VideoObjectItem.cpp \
	src/player/GStreamerPlayer.cpp \
	src/player/GStreamerMediaSample.cpp \
	src/player/GStreamerVideoRenderer.cpp \
	src/player/GStreamerSignalDispatcher.cpp \
	src/main/Application.cpp \
	src/main/main.cpp \
	src/videomaterial/VideoMaterial.cpp \
	src/videomaterial/VideoMaterialProviderGeneric.cpp

HEADERS += \
	src/base/ScopeGuard.hpp \
	src/base/VideoInputDevicesModel.hpp \
	src/base/SystemStats.hpp \
	src/base/FifoWatch.hpp \
	src/base/Utility.hpp \
	src/mesh/TeapotMesh.hpp \
	src/mesh/SphereMesh.hpp \
	src/mesh/TorusMesh.hpp \
	src/mesh/Mesh.hpp \
	src/mesh/CubeMesh.hpp \
	src/mesh/QuadMesh.hpp \
	src/scene/Arcball.hpp \
	src/scene/GLResources.hpp \
	src/scene/VideoObjectItem.hpp \
	src/scene/Camera.hpp \
	src/scene/Transform.hpp \
	src/scene/VideoObjectModel.hpp \
	src/player/GStreamerVideoRenderer.hpp \
	src/player/GStreamerPlayer.hpp \
	src/player/GStreamerMediaSample.hpp \
	src/player/GStreamerSignalDispatcher.hpp \
	src/player/GStreamerCommon.hpp \
	src/main/Application.hpp \
	src/videomaterial/VideoMaterial.hpp \
	src/videomaterial/VideoMaterialProviderGeneric.hpp


OTHER_FILES += src/main/UserInterface.qml

RESOURCES += src/main/Resources.qrc

INCLUDEPATH += src

isEmpty(PREFIX) {
	PREFIX = /usr/local
}
target.path = $$PREFIX/bin
INSTALLS += target

useImxV4L2 {
	DEFINES += USE_IMX_V4L2
}

vivante {
	DEFINES += WITH_VIV_GPU
	SOURCES += src/videomaterial/GLVIVDirectTextureExtension.cpp src/videomaterial/VideoMaterialProviderVivante.cpp
	HEADERS += src/videomaterial/GLVIVDirectTextureExtension.hpp src/videomaterial/VideoMaterialProviderVivante.hpp
}

QMAKE_CXXFLAGS += -Wextra -Wall -std=c++11 -pedantic -fPIC -DPIC -O0 -g3 -ggdb
QMAKE_LFLAGS += -fPIC -DPIC
