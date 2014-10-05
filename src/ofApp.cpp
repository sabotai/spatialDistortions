#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);

	    // Setup post-processing chain
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(true);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.createPass<VerticalTiltShifPass>()->setEnabled(false);
    post.createPass<GodRaysPass>()->setEnabled(false);
    post.createPass<ZoomBlurPass>()->setEnabled(false);



    post.createPass<RGBShiftPass>()->setAngle(329.f); //330 is nothing

    //post.createPass<RGBShiftPass>()->setEnabled(false);->setEnabled(false);

    post.createPass<RGBShiftPass>()->setEnabled(true);

	// enable depth->video image calibration
	kinect.setRegistration(true);

	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)

	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #

	// print the intrinsic IR sensor values
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}

#ifdef USE_TWO_KINECTS
	kinect2.init();
	kinect2.open();
#endif
/*
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
*/
	nearThreshold = 0;
	farThreshold = 1000;

	ofSetFrameRate(60);

	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);


    //ofEnableAlphaBlending();
    //ofEnableSmoothing();

	jump = 20;

	//oldTv.allocate(1920,1080);
/*
    dir.setDiffuseColor(ofColor(0.0f, 0.0f, 255.0f));
    dir.setSpecularColor(ofColor(255.0f, 0.0f, 0.0f));

    dir.setDirectional();
    dir_rot = ofVec3f(0, -95, 0);
    setLightOri(dir, dir_rot);
*/
    resolution = 2;
    grid = true;
}

//--------------------------------------------------------------
void ofApp::update() {
    ofSetWindowTitle(ofToString(nearThreshold) + " Near / " + ofToString(farThreshold) + " Far / " + ofToString(ofGetFrameRate()) + " FPS");

	ofBackground(0, 0, 0);

	kinect.update();


/*
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {

		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);

		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {

			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();

			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}

		// update the cv images
		grayImage.flagImageChanged();

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}

#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
*/
}

//--------------------------------------------------------------
void ofApp::draw() {

    //triangulation.reset();
		    // copy enable part of gl state
    glPushAttrib(GL_ENABLE_BIT);

    // setup gl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //light.enable();
	//ofSetDepthTest(true);
	ofDisableDepthTest();
    if (showSnow){
        snow();
    }
    // begin scene to post process
    ofEnableDepthTest();
    post.begin(easyCam);

    if (grid){
        ofPushMatrix();
            // the projected points are 'upside down' and 'backwards'
            ofTranslate(0,-800, 0);
            //ofRotateY(-30);
            ofRotateZ(90);
            ofDrawGridPlane(20000.f, 48.f, false);
        ofPopMatrix();
        //ofDrawAxis(1000.f);
    }

//oldTv.begin();

	//ofScale(2,2);
	ofSetColor(255, 255, 255);

    //easyCam.begin();
    drawPointCloud();
    //easyCam.end();


    ofNoFill();
    triangulation.draw();

    post.end();    // end scene and draw





    glPopAttrib();    // set gl state back to original


    ofDrawBitmapString(ofToString(easyCam.getDrag()), 20, 652);
    //oldTv.end();
    //oldTv.update();
    //oldTv.draw();
}

void ofApp::drawPointCloud() {
	int w = 640;
	int h = 480;
	ofMesh mesh;


	ofSetLineWidth(3);
	mesh.setMode(OF_PRIMITIVE_LINE_STRIP);

	for(int y = 0; y < h; y += resolution) {
		for(int x = 0; x < w; x += resolution) {
			if((kinect.getDistanceAt(x, y) > nearThreshold) && (kinect.getDistanceAt(x, y) < farThreshold)) {

                if (!showColor){
                    if (kinect.getDistanceAt(x,y) < farThreshold / 2){
                        mesh.addColor(kinect.getColorAt(x,y));
                        mesh.setMode(OF_PRIMITIVE_POINTS);
                    } else {
                        mesh.addColor(ofColor(ofRandom(0,220)));
                        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                    }

                 } else {
                    /*if (kinect.getDistanceAt(x,y) < farThreshold / 2){
                        mesh.setMode(OF_PRIMITIVE_POINTS);
                    } else {
                        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                    }*/
                        mesh.addColor(ofColor(255,20,20));
                }
                if (drawPoint){

                        mesh.setMode(OF_PRIMITIVE_POINTS);
                } else {

                        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                }

				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));

                //triangulation.addPoint(kinect.getWorldCoordinateAt(x,y));


			}
		}
	}
	glPointSize(5);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1800); // center the points a bit

   // dir.enable();

	mesh.drawVertices();
	//mesh.draw();
	//ofDisableDepthTest();

   // mVboBox = mesh;//.getMeshForIndices();

	//mVboBox.draw();

    //vbo.setMesh(mesh, GL_STATIC_DRAW);

    //vbo.draw(GL_QUADS,0,24);

	ofPopMatrix();
}


void ofApp::snow(){
    ofSetLineWidth(5);
    int noisePixel = 5;
    //ofLine(0,0,500,500);

    for (int yy = 0; yy < ofGetHeight() / noisePixel; yy++) {
      for (int xx = 0; xx < ofGetWidth() / noisePixel; xx++) {
        float noiseScale = 5 * ofGetFrameNum();

        float noiseVal = ofNoise(ofRandom(0,ofGetWidth())*noiseScale, ofRandom(0,ofGetHeight())*noiseScale);
        ofSetColor(noiseVal*255);

        ofSetLineWidth(noisePixel);
        ofLine(xx*noisePixel, yy*noisePixel, xx*noisePixel+noisePixel, yy*noisePixel+ofRandom(0,noisePixel));
      }
    }

}
//--------------------------------------------------------------
void ofApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();

#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {


	switch (key) {
	/*
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
*/
		case'p':
			drawPoint = !drawPoint;
			break;

		case'g':
			grid = !grid;
			break;

        case 't':
            //triangulation.addPoint(ofPoint(x,y));
            //triangulation.triangulate();
                break;

        case 's':
        case 'S':
            myImage.grabScreen(0,0,ofGetWidth(),ofGetHeight());
            //string
            myImage.saveImage("screencap " + ofToString(ofGetFrameNum() + ofRandom(0,200))+ ".png", OF_IMAGE_QUALITY_BEST);
            break;

		case '>':
		case '.':
			farThreshold += jump;
			if (farThreshold > 5000) farThreshold = 5000;
			break;

		case '<':
		case ',':
			farThreshold -= jump;
			if (farThreshold < 0) farThreshold = 0;
			break;

		case '+':
		case '=':
			nearThreshold += jump;
			if (nearThreshold > 5000) nearThreshold = 5000;
			break;

		case '-':
			nearThreshold -= jump;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
/*
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
*/
		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;

		case 'c':
		/*
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			*/
			showColor = !showColor;
			break;
		case 'w':
			showSnow = !showSnow;
			break;

        case 'r':
           // post.init(ofGetWidth(), ofGetHeight());
            break;

		case '1':
			fxaa = !fxaa;
			post.createPass<FxaaPass>()->setEnabled(fxaa);

			break;

		case '2':
		bloom = !bloom;
    post.createPass<BloomPass>()->setEnabled(bloom);
			break;

		case '3':
		dof = !dof;
    post.createPass<DofPass>()->setEnabled(dof);
			break;

		case '4':
		rgbpass = !rgbpass;
		post.createPass<RGBShiftPass>()->setEnabled(rgbpass);
		//kaleidoscope = !kaleidoscope;
    //post.createPass<KaleidoscopePass>()->setEnabled(kaleidoscope);
			break;

		case '5':
		noisewarp = !noisewarp;
		    post.createPass<NoiseWarpPass>()->setEnabled(noisewarp);
			break;

        case '6':
            pix = !pix;
            post.createPass<PixelatePass>()->setEnabled(pix);
            break;
        case '7':
            edge = !edge;
            post.createPass<EdgePass>()->setEnabled(edge);
            break;
        case '8':
		zoom = !zoom;
		post.createPass<ZoomBlurPass>()->setEnabled(zoom);
		/*
            tilt = !tilt;
    post.createPass<VerticalTiltShifPass>()->setEnabled(tilt);
    */
            break;
        case '9':
            god = !god;
    post.createPass<GodRaysPass>()->setEnabled(god);
            break;

		case '0':
			kinect.setLed(ofxKinect::LED_OFF);
			break;

		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;

		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;

		case OF_KEY_LEFT:
			if (resolution > 1){
                resolution--;
			}
			break;

		case OF_KEY_RIGHT:
			if (resolution < 10){
                resolution++;
			}
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
triangulation.addPoint(ofPoint(x,y, -1 * easyCam.getDistance()));
            triangulation.triangulate();
            }

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{}


void ofApp::setLightOri(ofLight &light, ofVec3f rot)
{
    ofVec3f xax(1, 0, 0);
    ofVec3f yax(0, 1, 0);
    ofVec3f zax(0, 0, 1);
    ofQuaternion q;
    q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
    light.setOrientation(q);
}
