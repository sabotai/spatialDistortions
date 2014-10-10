#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	//ofSetLogLevel(OF_LOG_VERBOSE);

	    // Setup post-processing chain

    post.init(ofGetWidth(), ofGetHeight());
    /*
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.createPass<VerticalTiltShifPass>()->setEnabled(false);
    post.createPass<GodRaysPass>()->setEnabled(false);
    post.createPass<ZoomBlurPass>()->setEnabled(false);
    */
    post.createPass<BloomPass>()->setEnabled(true);
    //post.createPass<RGBShiftPass>()->setEnabled(true);
    post.createPass<RGBShiftPass>()->setAngle(329.f); //330 is nothing

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
	colorThreshold = farThreshold / 2;

	//ofSetFrameRate(60);

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

    //ofSetBackgroundAuto(false);
    scaleAmt = 1.0;


	//threadedObject.start();



	//store the kinect width and height for convenience
	int width = 640;
	int height = 480;

    frameBlend = 40;

/*
	//add one vertex to the mesh for each pixel
	for (int y = 0; y < height; y++){
		for (int x = 0; x<width; x++){
			mainMesh.addVertex(ofPoint(x,y,0));	// mesh index = x + y*width
												// this replicates the pixel array within the camera bitmap...
			mainMesh.addColor(ofFloatColor(0,0,0));  // placeholder for colour data, we'll get this from the camera
		}
	}

	for (int y = 0; y<height-1; y++){
		for (int x=0; x<width-1; x++){
			mainMesh.addIndex(x+y*width);				// 0
			mainMesh.addIndex((x+1)+y*width);			// 1
			mainMesh.addIndex(x+(y+1)*width);			// 10

			mainMesh.addIndex((x+1)+y*width);			// 1
			mainMesh.addIndex((x+1)+(y+1)*width);		// 11
			mainMesh.addIndex(x+(y+1)*width);			// 10
		}
	}
	*/
}

//--------------------------------------------------------------
void ofApp::update() {
    ofSetWindowTitle(ofToString(nearThreshold) + " Near / " + ofToString(farThreshold) + " Far / " + ofToString(ofGetFrameRate()) + " FPS");



    ofBackground(0);




/*
ofSetColor(0,0,0, 5);
ofRect(0,0,ofGetWidth(), ofGetHeight());

ofSetColor(0,0,0);
*/
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
	//ofDisableDepthTest();

    ofScale(scaleAmt, scaleAmt); //resize the screen if switching between fullscreen
    //cout<< scaleAmt << endl;

    if (showSnow){
        snow();
    }



    // begin scene to post process
    //ofEnableDepthTest();
    post.begin(easyCam);

	ofSetColor(255, 255, 255);

    if (grid){ //draw the grid plane below the point cloud
        ofPushMatrix();
            // the projected points are 'upside down' and 'backwards'
            ofTranslate(0,-800, 0);
            //ofRotateY(-30);
            ofRotateZ(90);
            ofDrawGridPlane(20000.f, 48.f, false);
        ofPopMatrix();
        //ofDrawAxis(1000.f);
    }


    drawPointCloud();


    ofNoFill();
    //triangulation.draw(); //draw those random floating triangulations

    post.end();    // end scene and draw





    glPopAttrib();    // set gl state back to original




    ofDrawBitmapString(ofToString(easyCam.getDrag()), 20, 652);
    //oldTv.end();
    //oldTv.update();
    //oldTv.draw();

}

void ofApp::drawPointCloudAlt(){
/*
	int w = 640;
	int h = 480;

		for (int i=0; i< w * h; i++){

			//now we get the vertex aat this position
			//we extrude the mesh based on it's brightness
			ofVec3f tmpVec = mainMesh.getVertex(i);
			tmpVec.z = sampleColor.getBrightness() * extrusionAmount;
			mainMesh.setVertex(i, tmpVec);

			mainMesh.setColor(i, sampleColor);
		}
*/
}

void ofApp::drawPointCloud() {
	int w = 640;
	int h = 480;


	ofSetLineWidth(3);

	if (ofGetFrameNum()%frameBlend ==0){

mesh.clear();}
//mainMesh.clearIndices();


	for(int y = 0; y < h; y += resolution) {
		for(int x = 0; x < w; x += resolution) {
			if((kinect.getDistanceAt(x, y) > nearThreshold) && (kinect.getDistanceAt(x, y) < farThreshold)) {

/*
        //temp attempt to reduce the indices
                    mainMesh.addIndex(x+y*w);				// 0
                    mainMesh.addIndex((x+1)+y*w);			// 1
                    mainMesh.addIndex(x+(y+1)*w);			// 10

                    mainMesh.addIndex((x+1)+y*w);			// 1
                    mainMesh.addIndex((x+1)+(y+1)*w);		// 11
                    mainMesh.addIndex(x+(y+1)*w);			// 10
                    //
*/

                        int blend = 255  / (ofGetFrameRate() / frameBlend);

                if (!showColor){
                    if (kinect.getDistanceAt(x,y) > colorThreshold){

                        ofColor tempC(kinect.getColorAt(x,y));
                        tempC.a = blend;
                        mesh.addColor(tempC);
                        //mesh.addColor(kinect.getColorAt(x,y));

                        //mainMesh.setColor(x*y, kinect.getColorAt(x,y));
                        mesh.setMode(OF_PRIMITIVE_POINTS);
                    } else {
                        mesh.addColor(ofColor(ofRandom(0,220)));

                        //mainMesh.setColor(x*y, ofRandom(0,220));
                        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                    }

                 } else { //show red version

                        //ofColor rota(255,20,20);
                        mesh.addColor(ofColor(255,20,20, blend));
                        //mesh.addColor(rota);

                        //mainMesh.setColor()

                        //mainMesh.setColor(x*y, ofColor(255,20,20));
                }
                if (drawPoint){
                        mesh.setMode(OF_PRIMITIVE_POINTS);
                } else {
                        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                }

				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));

                //mainMesh.setVertex((x*y), kinect.getWorldCoordinateAt(x,y));
				//path.lineTo(kinect.getWorldCoordinateAt(x,y));
				//poly.addVertex(kinect.getWorldCoordinateAt(x,y));

                //triangulation.addPoint(kinect.getWorldCoordinateAt(x,y));
			} else {

                        //otherMesh.addColor(kinect.getColorAt(x,y));
				//otherMesh.addVertex(ofPoint(x, y, 5000));
			}
		}
	}


	//}
	glPointSize(10);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(1, -1, -1); //flip it back around



	//    otherMesh.drawVertices();
	ofTranslate(0, 0, -1800); // center the points a bit
    //mainMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    //mainMesh.drawFaces();
    //mesh.drawVertices();
    mesh.drawFaces();
	ofPopMatrix();
}


void ofApp::snow(){
	//threadedObject.draw();

    int noisePixel = 5;
    //ofLine(0,0,500,500);

    for (int yy = 0; yy < ofGetHeight() / noisePixel; yy++) {
      for (int xx = 0; xx < ofGetWidth() / noisePixel; xx++) {
        //float noiseScale = 5 * ofGetFrameNum();

        //float noiseVal = ofNoise(ofRandom(0,ofGetWidth())*noiseScale, ofRandom(0,ofGetHeight())*noiseScale);
        //ofSetColor(noiseVal*255);

        int alphaAmt = 200;
        ofSetColor(ofRandom(0,255), ofRandom(alphaAmt, 255));

        ofSetLineWidth(noisePixel);
        //ofLine(xx*noisePixel, yy*noisePixel, xx*noisePixel+noisePixel, yy*noisePixel+ofRandom(0,noisePixel));
        otherMesh.addColor(ofColor(ofRandom(0,255)));
        otherMesh.addVertex(ofPoint(xx,yy));
      }
    }



}
//--------------------------------------------------------------
void ofApp::exit() {
	//kinect.setCameraTiltAngle(0); // zero the tilt on exit
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

		case '[':
			colorThreshold += jump;
			if (colorThreshold > farThreshold) colorThreshold = farThreshold;
			break;

		case ']':
			colorThreshold -= jump;
			if (colorThreshold < 0) colorThreshold = 0;
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
		case 'f':
            scaleAmt = float(ofGetScreenWidth()) / float(ofGetWidth());
			ofToggleFullscreen();
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
            //triangulation.addPoint(ofPoint(x,y, -1 * easyCam.getDistance()));
            //triangulation.triangulate();
            }

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{}

/*
void ofApp::setLightOri(ofLight &light, ofVec3f rot)
{
    ofVec3f xax(1, 0, 0);
    ofVec3f yax(0, 1, 0);
    ofVec3f zax(0, 0, 1);
    ofQuaternion q;
    q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
    light.setOrientation(q);
}
*/
