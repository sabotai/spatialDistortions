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
    //post.createPass<RGBShiftPass>()->setAngle(329.f); //330 is nothing

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

	nearThreshold = 0;
	farThreshold = 2900;//5000;
	colorThreshold = 2500;//farThreshold / 2;

	ofSetFrameRate(60);

	// set the tilt on startup
	angle = 14;
	kinect.setCameraTiltAngle(angle);

	jump = 20;

    resolution = 1;
    grid = true;

    scaleAmt = 1.0; //default scale to 1, used later for fullscreen


	//threadedObject.start();


    frameBlend = 1;

    oldComp = 0;
    compAverage = 0;

    staticSound.loadSound("static.wav");
    staticSound.setVolume(0.1f);
    staticSound2.loadSound("static2.wav");
    staticSound2.setVolume(0.1f);


    img.allocate(640, 480, OF_IMAGE_COLOR_ALPHA);

}

//--------------------------------------------------------------
void ofApp::update() {
    ofSetWindowTitle(ofToString(nearThreshold) + " Near / " + ofToString(farThreshold) + " Far / " + ofToString(colorThreshold) + " colodThresh " + ofToString(resolution) + " resolution" + ofToString(ofGetFrameRate()) + " FPS") ;


    ofBackground(0);
    kinect.update();



}

//--------------------------------------------------------------
void ofApp::draw() {


    //triangulation.reset();
		    // copy enable part of gl state
    glPushAttrib(GL_ENABLE_BIT);
    // setup gl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    ofScale(scaleAmt, scaleAmt); //resize the screen if switching between fullscreen
    //cout<< scaleAmt << endl;

    // begin scene to post process
    //ofEnableDepthTest();
    post.begin(easyCam);

	ofSetColor(255, 255, 255);

    if (grid){ //draw the grid plane below the point cloud
        ofPushMatrix();
            // the projected points are 'upside down' and 'backwards'
            ofTranslate(0,-800, 0);
            ofRotateZ(90);
            ofDrawGridPlane(20000.f, 48.f, false);
        ofPopMatrix();
    }


    drawPointCloud();

    //triangulation.draw(); //draw those random floating triangulations

    post.end();    // end scene and draw



    glPopAttrib();    // set gl state back to original




    if (showSnow){
        glEnable(GL_DEPTH);
        snow();
    }

}


void ofApp::drawPointCloud() {
	int w = 640;
	int h = 480;

	float tempCompAverage;

    ofMesh mesh;



	ofSetLineWidth(3);


    int percentCount = 0;
    int totalRuns = 0;
    int whichStatic = 1;

    int blend;

    if (frameBlend > 1){
        blend = 255  / (frameBlend / ofGetFrameRate());
        blend = ofClamp(blend, 0, 255);
        //cout << blend << endl;
    } else {
        blend = 255;
    }



	for(int y = 0; y < h; y += resolution) {
		for(int x = 0; x < w; x += resolution) {


			if((kinect.getDistanceAt(x, y) > nearThreshold) && (kinect.getDistanceAt(x, y) < farThreshold)) {
                totalRuns++;
               mesh.setMode(OF_PRIMITIVE_POINTS); //default to points


                if (!showColor){
                    if (kinect.getDistanceAt(x,y) > colorThreshold){

                        percentCount++;
                        tempC = ofColor((kinect.getColorAt(x,y)));
                        //tempC.a = blend;
                        mesh.addColor(tempC);
                        //mesh.addColor(kinect.getColorAt(x,y));

                        //mainMesh.setColor(x+y * w, kinect.getColorAt(x,y));
                        mesh.setMode(OF_PRIMITIVE_POINTS);
                        whichStatic = 1;
                    } else {
                        percentCount--;
                        mesh.addColor(ofColor(ofRandom(0,200)));

                        //mainMesh.setColor(x+y * w, ofRandom(0,220));
                        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                        whichStatic = 2;
                    }

                 } else { //show red version

                        //ofColor rota(255,20,20);
                        mesh.addColor(ofColor(255,20,20));//, blend));
                        //mesh.addColor(rota);

                        //mainMesh.setColor()

                        //mainMesh.setColor(x+y * w, ofColor(255,20,20));
                }

                if (defineMeshMode){ //change between glitchy auto mode and user defined mesh mode
                    if (drawPoint){
                            mesh.setMode(OF_PRIMITIVE_POINTS);
                    } else {
                            mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
                    }
                }

				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));

                //triangulation.addPoint(kinect.getWorldCoordinateAt(x,y));

}

    ////////////////////////////////// BEGIN Z AXIS SHIFT //////////////////////////////
        //if these pixels change in depth more than the threshold, record their average movement to adjust the camera

        float comp = kinect.getDistanceAt(x,y) - lastCoord[x+y*w];
        int compThresh = 100 / resolution;

        tempCompAverage += comp;
        compCount +=1;
        lastCoord[x+y*w] = kinect.getDistanceAt(x, y);
        //cout << lastCoord[x+y*w] << endl;

		}
	} //close out the big for loop

    int compMin = 2000 / resolution; //number of points needed to even compare them
    compFrameCount++;

    if(compCount > compMin){
        tempCompAverage = tempCompAverage / compCount;
    } else {
        tempCompAverage = 0;
    }

    transAmt = 0;

    if (compFrameCount < ofGetFrameRate()/2){ //every half second
        compAverage += tempCompAverage;
        transAmt = oldComp;

    } else {
        compAverage /= compFrameCount;

        transAmt = oldComp;

        //cout<< "compAvg is " << compAverage << "  and oldComp is " << oldComp <<  " and lerped is " << compLerp << endl;
        //compAverage = oldComp;
        //compAverage *= 100; //multiplier for distance changes
        if (compAverage >= -10000000){ //this prevents nan
            oldComp += compAverage;}
        compAverage = 0;
        compFrameCount = 0;
    }

    //cout<< "transamt is " << transAmt << endl;
    ofTranslate(0, 0, transAmt * 40); // move the camera by the lerped average change in depth above a threshold

    if (transAmt > -1000000 && transAmt != 0){
        staticSound.setSpeed(ofMap(transAmt, 0, 250,0,2.0));
        staticSound2.setSpeed(ofMap(transAmt, 0, 250,0,2.0));
    }

    tempCompAverage = 0;
    compCount = 0;
    //compAverage = 0;

    ////////////////////////////////// END Z AXIS MOVEMENT //////////////////////////////

    //only play the static sound when less than half the screen shows rgb
    //if (percentCount < 0){


    // change the sound sample... originally, i was using the same soundplayer object and loading the other file, but this is super slow
        if (whichStatic == 1){
            if (!staticSound.getIsPlaying()){
                staticSound.play();
            }

            if (staticSound2.getIsPlaying()){
                staticSound2.stop();}

        } else {
            if (!staticSound2.getIsPlaying()){
                staticSound2.play();
            }

            if (staticSound.getIsPlaying()){
                staticSound.stop();}
        }
    if (percentCount != 0){
    staticSound.setVolume(ofMap(percentCount, -totalRuns, totalRuns, 0.7f,  0.0f));
    staticSound2.setVolume(ofMap(percentCount, -totalRuns, totalRuns, 0.7f,  0.0f));
    }
    percentCount = 0; //reset for the next loop

    ////////////////////////////////// BEGIN PIXEL INDEX BUILDING //////////////////////////////
    //if you start this when there are too many pixels, it will freeze

    if (collectIndices){
            // Don't forget to change to lines mode!
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);

        // ...
        // Omitting the code for creating vertices for clarity
        // but don't erase it from your file!

        // Let's add some lines!
            float connectionDistance = 200;
            //mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
            int numVerts = mesh.getNumVertices();
            for (int a=0; a<numVerts; ++a) {
                ofVec3f verta = mesh.getVertex(a);
                for (int b=a+1; b<numVerts; ++b) {
                    ofVec3f vertb = mesh.getVertex(b);
                    float distance = verta.distance(vertb);
                        if (distance <= connectionDistance) {
                            mesh.addIndex(a);
                            mesh.addIndex(b);
                        }
                    }



            }

    }
    ////////////////////////////////// END PIXEL INDEX BUILDING //////////////////////////////



	glPointSize(3 * resolution); //larger points when there are less of them
	ofPushMatrix();
        // the projected points are 'upside down' and 'backwards'
        ofScale(1, -1, -1); //flip it back around
        ofTranslate(0, 0, -1800); // center the points a bit
        mesh.draw();
	ofPopMatrix();
}


void ofApp::snow(){

    for (int i = 0; i < img.getPixelsRef().size(); i+=4){

            int rando = ofRandom(0,255);
            img.getPixelsRef()[i] = rando;//ofColor(rando, rando, rando, 100) ; // make some op-art
            img.getPixelsRef()[i+1] = rando;
            img.getPixelsRef()[i+2] = rando;
            img.getPixelsRef()[i+3] = ofMap(transAmt, 200, 500, 255, 200); //alpha

        }
    img.reloadTexture();
    img.draw(0,0,0, ofGetWidth(), ofGetHeight());

}
//--------------------------------------------------------------
void ofApp::exit() {
	//kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();

}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {


	switch (key) {

		case ' ':
            collectIndices = !collectIndices;
			break;

		case'p':
			drawPoint = !drawPoint;

            staticSound.play();
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
			if (nearThreshold > 10000) nearThreshold = 10000;
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
			//kinect.setCameraTiltAngle(angle); // go back to prev tilt
			//kinect.open();
			defineMeshMode = !defineMeshMode;
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
			cout << "camera angle is " << angle << endl;
			break;

		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			cout << "camera angle is " << angle << endl;
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
