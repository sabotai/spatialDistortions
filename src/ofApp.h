#pragma once

#include "ofMain.h"
//#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxPostProcessing.h"
#include "ofxDelaunay.h"
#include "threadedObject.h"

class ofApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();
	void snow();

	void drawPointCloud();

	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	ofxKinect kinect;


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

    ofxDelaunay triangulation;

    bool fxaa, bloom, dof, kaleidoscope, noisewarp, pix, edge, tilt, god, rgbpass, zoom;


    bool showColor, drawPoint, grid, collectIndices, defineMeshMode;

    float scaleAmt;


	ofMesh mesh, otherMesh, mainMesh;


    ThreadedObject threadedObject;

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
