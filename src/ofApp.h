#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"

#define HOST "localhost"
#define PORT 10000

//--------------------------------------------------------
class ofApp : public ofBaseApp {
    
public:
    
    
    vector <int> directions;
    
    //ScreenGrab Stuff
    int captureWidth, captureHeight;
    
    ofImage image;
    ofxCvHaarFinder finder;
    
    // ofxCV stuff
    ofxCv::ContourFinder contourFinder;
	float threshold;
	ofxCv::TrackingColorMode trackingColorMode;
	ofColor targetColor;
    cv::Point2f blob;
    
    
    //OpenCV Stuff
    
    ofxCvColorImage colorImg;
    ofImage img;
    ofPixels pixels;
    
    float myArea;
    float myArea2;
    ofPoint myCentroid;
    
    bool debug;
    
    float startTimer;
    float timer;
    
    bool isScanning;
    bool isFlying;
    bool isTracking;
    bool forward;
    int num;
    float s;
    
    void checkContours();
    void scanning();
    cv::Point2f getCenterRect();
    
    // void tracking(cv::Rect myBlob);
    void trackingCentroid(cv::Point2f blob);
    void sendOsc();
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofTrueTypeFont font;
    ofxOscSender sender;
    
};

