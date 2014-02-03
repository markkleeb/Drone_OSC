#include "ofApp.h"
using namespace cv;
using namespace ofxCv;


extern "C" {
#include "macGlutfix.h"
}

//--------------------------------------------------------------
void ofApp::setup(){
    
    timer = ofGetElapsedTimeMillis();
    startTimer = ofGetElapsedTimeMillis();
    
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    
	
    isTracking = false;
    isArrived = false;
    isScanning = false;
    
    myArea = 0;
    num = 0;
    
    for(int i=0; i < 12; i++){
        directions.push_back(0);
    }
    
	// open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);



    //ScreenGrab--------------------------------
    captureWidth = ofGetWidth();
    captureHeight = ofGetHeight();
    image.allocate(captureWidth, captureHeight, OF_IMAGE_COLOR);
    //pixels.allocate(captureWidth, captureHeight, OF_IMAGE_COLOR);
    
    //cv
	contourFinder.setMinAreaRadius(10);
	contourFinder.setMaxAreaRadius(200);
	trackingColorMode = TRACK_COLOR_RGB;
    
    
    threshold = 20;
    targetColor = ofColor(255, 0, 0);
    contourFinder.setTargetColor(targetColor, trackingColorMode);
    
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    //ScreenGrabStuff-----------------------------------------
    captureWidth = ofGetWidth();
    captureHeight = ofGetHeight();
    
    unsigned char * data = pixelsBelowWindow(ofGetWindowPositionX(), ofGetWindowPositionY(), captureWidth, captureHeight);
    
    // now, let's get the R and B data swapped, so that it's all OK:
    for (int i = 0; i < captureWidth * captureHeight; i++){
        
        unsigned char r = data[i*4]; // mem A
        
        data[i*4]   = data[i*4+1];
        data[i*4+1] = data[i*4+2];
        data[i*4+2] = data[i*4+3];
        data[i*4+3] = r;
    }
    
    
    if (data!= NULL) {
        //tex.loadData(data, captureWidth, captureHeight, GL_RGBA);
        //tex.readToPixels(pixels);
        //image = pixels;
        image.setFromPixels(data, captureWidth, captureHeight, OF_IMAGE_COLOR_ALPHA, true);
        image.setImageType(OF_IMAGE_COLOR);
        image.update();
        finder.findHaarObjects(image.getPixelsRef());
        
        
        
        
		contourFinder.setThreshold(threshold);
		contourFinder.findContours(image);
        
        
        
        //blob = getCenterRect();
        //  if (!isScanning){
        //trackingCentroid(blob);
        // }
        
    }
    else {return;}
    //cout << imageBelowWindow()[0] << endl;
    
    if(isFlying && ofGetElapsedTimeMillis() - startTimer > 8000){
        scanning();
    }
    
    
    
    if(isTracking){
        
        trackingCentroid(getCenterRect());
    }
    
   
    
    if(isArrived){
        arrived();
    }
    
    if(ofGetElapsedTimeMillis() - timer > 100){
    
        sendOsc();
        timer = ofGetElapsedTimeMillis();
    }
    
    
}

//--------------------------------------------------------------
void ofApp::draw(){
	
    /*
    // display instructions
	string buf;
	buf = "sending osc messages to" + string(HOST) + ofToString(PORT);
	ofDrawBitmapString(buf, 10, 20);
	ofDrawBitmapString("move the mouse to send osc message [/mouse/position <x> <y>]", 10, 50);
	ofDrawBitmapString("click to send osc message [/mouse/button <button> <\"up\"|\"down\">]", 10, 65);
	ofDrawBitmapString("press A to send osc message [/test 1 3.5 hello <time>]", 10, 80);
     */
    
    
    //ScreenGrabStuff-------------------------------------------
    ofSetColor(255);
    
	image.draw(0, 0);
	
    //draw the blob
    ofSetColor(255, 0, 0);
    ofNoFill();
    ofSetLineWidth(5);
    ofEllipse(getCenterRect().x, getCenterRect().y, 50, 50);
    
    ofSetColor(255, 255, 255);
    ofFill();
    ofEllipse(ofGetWindowWidth()/2, ofGetWindowHeight()/2, 10, 10);
    ofNoFill();
    ofSetLineWidth(5);
    ofRect(ofGetWindowWidth()/2-100, ofGetWindowHeight()/2-100, 200, 200);
    ofSetLineWidth(1);
    
    if(debug == true){
        
        
        //Contour Finder
        drawHighlightString("Flying? " + ofToString(isFlying), 10, 100);
        drawHighlightString("Scanning? " + ofToString(isScanning), 10, 120);
        drawHighlightString("Tracking? " + ofToString(isTracking), 10, 140);
        drawHighlightString("Arrived? " + ofToString(isArrived), 10, 160);
        drawHighlightString("Area " + ofToString(myArea), 10, 180);

        
        drawHighlightString("Up " + ofToString(directions[0]), ofGetWindowWidth()-120, 100);
        drawHighlightString("Down " + ofToString(directions[1]), ofGetWindowWidth()-120, 120);
        drawHighlightString("Left " + ofToString(directions[2]), ofGetWindowWidth()-120, 140);
        drawHighlightString("Right " + ofToString(directions[3]), ofGetWindowWidth()-120, 160);
        drawHighlightString("Forward " + ofToString(directions[4]), ofGetWindowWidth()-120, 180);
        drawHighlightString("Backward " + ofToString(directions[5]), ofGetWindowWidth()-120, 200);
        drawHighlightString("Clockwise " + ofToString(directions[6]), ofGetWindowWidth()-120, 220);
        drawHighlightString("Counterclockwise " + ofToString(directions[7]), ofGetWindowWidth()-120, 240);
        
        
        
        


        
        contourFinder.draw();
        drawHighlightString(ofToString((int) ofGetFrameRate()) + " fps", 10, 300);
        drawHighlightString(ofToString((int) threshold) + " threshold", 10, 320);
        drawHighlightString(trackingColorMode == TRACK_COLOR_HSV ? "HSV tracking" : "RGB tracking", 10, 340);
        
        
        ofTranslate(10, 360);
        ofFill();
        ofSetColor(0);
        ofRect(-3, -3, 64+6, 64+6);
        ofSetColor(targetColor);
        ofRect(0, 0, 64, 64);
        ofNoFill();
        
        
        
    }
    for(int i = 0; i < contourFinder.getBoundingRects().size(); i++){
        ofSetColor(127);
        
        //ofCircle(  Point<float>(contourFinder.getCentroid(i)), 25);
        
    }
}

//-----------------------------------------------------------
void ofApp::sendOsc(){
    
    ofxOscMessage m;
    m.setAddress("/drone");
    for(int i=0; i < directions.size(); i++){
        m.addIntArg(directions[i]);
    }
   
    sender.sendMessage(m);

    
}

//--------------------------------------------------------------
void ofApp::scanning(){
    
    
    
    
    //drone.controller.spinSpeed += s;
   
    checkContours();
    

    }

//----------------------------------------------------------------

cv::Point2f ofApp::getCenterRect(){
    
    
    myArea2 = 0;
    for(int i = 0; i < contourFinder.getBoundingRects().size(); i++){
        
        float area = contourFinder.getBoundingRects()[i].width*contourFinder.getBoundingRects()[i].height;
        // cout << "Area #" << i << " = " << area << endl;
        if(area > myArea2){
            myArea2 = area;
            
            num = i;
            
        }
        
    }
    
    // cout << "Which is biggest? " << num <<" , "<<contourFinder.getBoundingRects().size()<<" is the number of rects"<<endl;
    
    if(contourFinder.getBoundingRects().size() !=0 ){
        return contourFinder.getCenter(num);
    }
    
}
//----------------------------------------------------------------

void ofApp::trackingCentroid(cv::Point2f blobCoordinates){
    
    
    if( getCenterRect().x > ofGetWindowWidth()/2-200 && getCenterRect().x < ofGetWindowWidth()/2+200 && getCenterRect().y > ofGetWindowHeight()/2 - 200 && getCenterRect().y < ofGetWindowHeight()/2 + 200){
     
        directions[4] = 1;
        directions[10] = 0;
        
        forward = true;
    }
    
    else{
        forward = false;
        directions[4] = 0;
        directions[10] = 0;
    }
    
    if(getCenterRect().x > ofGetWindowWidth()/2+100) {
        
        directions[3] = 1;
        directions[2] = 0;
        directions[10] = 0;

        
    }
    
    else if(getCenterRect().x < ofGetWindowWidth()/2-100) {
        
        directions[2] = 1;
        directions[3] = 0;
        directions[10] = 0;

        
    }
    
    else {
        directions[2] = 0;
        directions[3]=0;
        directions[10] = 0;
    }
    
    if(getCenterRect().y > ofGetWindowHeight()/2+100) {
        
        directions[1] = 1;
        directions[0] = 0;
        directions[10] = 0;
    }
    
    else if(getCenterRect().y < ofGetWindowHeight()/2-100) {
        
        directions[0] = 1;
        directions[1] = 0;
        directions[10] = 0;
        
    }
    
    else {
        
        directions[0] = 0;
        directions[1] = 0;
        directions[10] = 0;

    }
    
    
    // drone.controller.pitchAmount = 0;
    
    //checkContours();
    
    
    
    
}
//--------------------------------------------------------------

void ofApp::checkContours(){
    
    
    myArea = 0;
    for(int i = 0; i < contourFinder.getBoundingRects().size(); i++){
        
        float area = contourFinder.getBoundingRects()[i].width*contourFinder.getBoundingRects()[i].height;
        if(area > myArea){
            myArea = area;
            
        }
        
        if(myArea > 5000 && myArea < 100000){
            
            isScanning = false;
            isTracking = true;
            isArrived = false;
        }
        
        else if(myArea > 100000 ){
            isScanning = false;
            isTracking = false;
            isArrived = true;
        }
        
        else{
            isScanning = true;
            isTracking = false;
            isArrived = false;
        }
        
    }
    
}

//--------------------------------------------------------------

void ofApp::arrived(){
    
    for(int i=0; i<directions.size(); i++){
        directions[i] = 0;
    }
    
    directions[10] = 1;
    
    
    
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	/*
    if(key == 'a' || key == 'A'){
		ofxOscMessage m;
		m.setAddress("/pilot");
		m.addStringArg("'/pilot/drone', {action: 'takeoff'}");
		sender.sendMessage(m);
	}
     */
   
    switch(key){
             

        case 'e':
            
            directions[11] = 1;
            
            
            
            break;

            
     case 't':
           
            directions[8] = 1;
            directions[10] = 0;
            
            
            break;
        
        case 'l':
           
            directions[9] = 1;
           directions[10] = 0;
            break;
        
        case 'w':
            
            directions[0] = 1;
            directions[10] = 0;
        break;
        case 'a':
            
            directions[2]=1;
            directions[10] = 0;
            break;
        case 's':
            
            directions[1] = 1;
            directions[10] = 0;
            break;
        case 'd':
            
            directions[3] = 1;
            directions[10] = 0;
            break;
            
        case OF_KEY_UP:
            
            directions[4] = 1;
            directions[10] = 0;
            break;
            
        case OF_KEY_DOWN:
            
            directions[5] = 1;
            directions[10] = 0;
            break;
        case OF_KEY_LEFT:
            
            directions[7] = 1;
            directions[10] = 0;
            break;
        case OF_KEY_RIGHT:
            
            directions[6] = 1;
            directions[10] = 0;
            break;

    }
    }

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    switch(key){
            
            
        case 'e':
            
            directions[11] = 0;
            break;
            
        case ' ': debug = !debug;
            break;
        case 't':
            
            directions[8] = 0;
            isFlying = true;
            startTimer = ofGetElapsedTimeMillis();
            
            break;
            
        case 'l':
            
            directions[9] = 0;
            isTracking = false;
            isScanning = false;
            isFlying = false;
            
            for(int i=0; i<directions.size(); i++){
                directions[i] = 0;
            }
            break;
            
        case 'w':
            
            directions[0] = 0;
            directions[10] = 1;
            
            break;
        case 'a':
            
            directions[2]=0;
            directions[10] = 1;
            
            break;
        case 's':
            
            directions[1] = 0;
            directions[10] = 1;
            
            break;
        case 'd':
            
            directions[3] = 0;
            directions[10] = 1;
            
            break;
            
        case OF_KEY_UP:
            
            directions[4] = 0;
            directions[10] = 1;
            
            break;
            
        case OF_KEY_DOWN:
            
            directions[5] = 0;
            directions[10] = 1;
            
            break;
        case OF_KEY_LEFT:
            
            directions[7] = 0;
            directions[10] = 1;
            
            break;
        case OF_KEY_RIGHT:
            
            directions[6] = 0;
            directions[10] = 1;
            
            break;
        case '=':
            threshold = threshold + 1;
            break;
        case '-':
            threshold = threshold - 1;
            break;
    }
    

    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    targetColor = image.getPixelsRef().getColor(x, y);
	contourFinder.setTargetColor(targetColor, trackingColorMode);
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

