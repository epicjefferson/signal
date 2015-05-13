#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxLeapMotion2.h"
#include "ofxAudioDecoder.h"

#define HOST "localhost"
#define PORT 44000


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
//    void drawTriangle(vector <ofxLeapMotionSimpleHand> simpleHands);
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
    
    //leap
    ofxLeapMotion leap;
    vector <ofxLeapMotionSimpleHand> simpleHands;
    vector <Hand> hands;
    vector <int> fingersFound;
    
    //openframeworks cam and light
    ofEasyCam cam;
    ofLight l1;
    ofLight l2;
    ofMaterial m1;
    ofMesh left, right, triangle, selectionArea;
    
    //OSC
    ofxOscSender oscSender;
    bool senderSwitch = false;
    
    //audio
    void loadFile(string filename);
    ofxAudioDecoder audio;
    
};

const string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};
