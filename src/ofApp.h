#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxPostProcessing.h"
#include "ofxOldTv.h"
#include "ofxDelaunay.h"
#include "threadedObject.h"

// Windows users:
// You MUST install the libfreenect kinect drivers in order to be able to use
// ofxKinect. Plug in the kinect and point your Windows Device Manager to the
// driver folder in:
//
//     ofxKinect/libs/libfreenect/platform/windows/inf
//
// This should install the Kinect camera, motor, & audio drivers.
//
// You CANNOT use this driver and the OpenNI driver with the same device. You
// will have to manually update the kinect device to use the libfreenect drivers
// and/or uninstall/reinstall it in Device Manager.
//
// No way around the Windows driver dance, sorry.

// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS

class ofApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();
	void snow();

	void drawPointCloud();
	void drawPointCloudAlt();

	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    //void setLightOri(ofLight &light, ofVec3f rot);
	ofxKinect kinect;

#ifdef USE_TWO_KINECTS
	ofxKinect kinect2;
#endif
/*
	ofxCvColorImage colorImg;

	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image

	ofxCvContourFinder contourFinder;

	bool bThreshWithOpenCV;
	bool bDrawPointCloud;
	*/

	bool showSnow;
	string displayMode;

	int nearThreshold;
	int farThreshold;
	int colorThreshold;
	int resolution;

	int angle;

	int jump;

	// used for viewing the point cloud
	ofEasyCam easyCam;

    ofxPostProcessing post;

    ofImage myImage;

/*
    ofVboMesh	mVboBox;
    ofVbo vbo;
    ofPath path;
    ofPolyline poly;

    ofxOldTv oldTv;
    ofLight dir;
    ofVec3f dir_rot;

    */
        ofxDelaunay triangulation;

    bool fxaa, bloom, dof, kaleidoscope, noisewarp, pix, edge, tilt, god, rgbpass, zoom;


    bool showColor, drawPoint, grid, collectIndices;

    float scaleAmt;


	ofMesh mesh, otherMesh, mainMesh;


    ThreadedObject threadedObject;
        ///< \brief Our threaded object.

        int frameBlend;
        ofColor tempC;
        float lastCoord [640 * 480]; //save the last depth for comparison
        float compCount, compFrameCount;
        float compAverage, oldComp;

        float compLerp;


		ofSoundPlayer 		staticSound, staticSound2;
        ofImage img;
		//ofImage snowTexture;
		//unsigned char * pixels;

		float transAmt;
};
