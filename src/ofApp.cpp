#include "ofApp.h"

float exis, leftSelection, rightSelection;

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(120);
//    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    leap.open();
    
    cam.setOrientation(ofPoint(-20, 0, 0));
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    //setup OSC
    oscSender.setup(HOST, PORT);
    
    //load audio file
    loadFile("demo.mp3");
    
    //smooth lines
    ofEnableSmoothing();
}

//--------------------------------------------------------------
void ofApp::update(){
    fingersFound.clear();
    
    simpleHands = leap.getSimpleHands();

    // if sender switch is set to 1, send all this leap data over OSC
    if(senderSwitch){

        hands = leap.getLeapHands();
        
        if (hands.size() > 0) {
            for (int h = 0; h < hands.size(); h++){
                
                ofxOscMessage m;
                
                // Get the current hand
                Hand & hand = hands[h];
                
                string handType = hand.isLeft() ? "Left" : "Right";     //pretty sweet way of knowing which hand
                
                m.setAddress("/" + handType);
                m.addStringArg("/palm");
                m.addFloatArg(hand.palmPosition()[0]);      // X
                m.addFloatArg(hand.palmPosition()[1]);      // Y
                m.addFloatArg(hand.palmPosition()[2]);      // Z
                oscSender.sendMessage(m);
                m.clear();
                
                
                //get grab Strength
                m.setAddress("/" + handType);
                m.addStringArg("/grabStrength");
                m.addFloatArg(hand.grabStrength());
                oscSender.sendMessage(m);
                m.clear();
                
                // Calculate the hand's pitch, roll, and yaw angles
                const Vector normal = hand.palmNormal();
                const Vector direction = hand.direction();
                
                m.setAddress("/" + handType);
                m.addStringArg("/pitch");
                m.addFloatArg(direction.pitch() * RAD_TO_DEG);
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/roll");
                m.addFloatArg(normal.roll() * RAD_TO_DEG);
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/yaw");
                m.addFloatArg(direction.yaw() * RAD_TO_DEG);
                oscSender.sendMessage(m);
                m.clear();
                
                
                // Get the Arm bone
                Arm arm = hand.arm();
                
                m.setAddress("/" + handType);
                m.addStringArg("/armDirection");
                m.addFloatArg(arm.direction()[0]);       // x
                m.addFloatArg(arm.direction()[1]);       // y
                m.addFloatArg(arm.direction()[2]);       // z
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/wristPosition");
                m.addFloatArg(arm.wristPosition()[0]);       // x
                m.addFloatArg(arm.wristPosition()[1]);       // y
                m.addFloatArg(arm.wristPosition()[2]);       // z
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/elbowPosition");
                m.addFloatArg(arm.elbowPosition()[0]);       // x
                m.addFloatArg(arm.elbowPosition()[1]);       // y
                m.addFloatArg(arm.elbowPosition()[2]);       // z
                oscSender.sendMessage(m);
                m.clear();
                
                
                // Get fingers
                const FingerList fingers = hand.fingers();
                for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
                    const Finger finger = *fl;
                    
                    m.setAddress("/" + handType);
                    m.addStringArg("/fingers");
                    m.addStringArg("/" + fingerNames[finger.type()]);
                    m.addStringArg("/tip");
                    m.addFloatArg(finger.tipPosition()[0]) ;       // x
                    m.addFloatArg(finger.tipPosition()[1]) ;       // Y
                    m.addFloatArg(finger.tipPosition()[2]) ;       // Z
                    oscSender.sendMessage(m);
                    m.clear();
                    
                    // Get finger bones
                    for (int b = 0; b < 4; ++b) {
                        Bone::Type boneType = static_cast<Bone::Type>(b);
                        Bone bone = finger.bone(boneType);
                        
                        //sending current bones' previous joint
                        m.setAddress("/" + handType);
                        m.addStringArg("/fingers");
                        m.addStringArg("/" + fingerNames[finger.type()]);
                        m.addStringArg("/" + boneNames[boneType]);
                        m.addStringArg("/prevJoint");
                        m.addFloatArg(bone.prevJoint()[0]);       // x
                        m.addFloatArg(bone.prevJoint()[1]);       // y
                        m.addFloatArg(bone.prevJoint()[2]);       // z
                        oscSender.sendMessage(m);
                        m.clear();
                        
                        //sending current bones' next joint
                        m.setAddress("/" + handType);
                        m.addStringArg("/fingers");
                        m.addStringArg("/" + fingerNames[finger.type()]);
                        m.addStringArg("/" + boneNames[boneType]);
                        m.addStringArg("/nextJoint");
                        m.addFloatArg(bone.nextJoint()[0]);       // x
                        m.addFloatArg(bone.nextJoint()[1]);       // y
                        m.addFloatArg(bone.nextJoint()[2]);       // z
                        oscSender.sendMessage(m);
                        m.clear();
                        
                    }
                    
                }
                
            }
        }
    }
    else{
        //if we're not sending out all leap data,
        //just use the triangle (left hand) + centroid
        //and selection (right hand)
        
        if( leap.isFrameNew() && simpleHands.size() ){
            
            ofxOscMessage m;    // setup OSC message
            
            leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
            leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
            leap.setMappingZ(-150, 150, -200, 200);
            
            fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
            
            float centroidX = 0.0, centroidY = 0.0, centroidZ = 0.0;  //accumulators for calculating centroid
            
            for(int i = 0; i < simpleHands.size(); i++){
                bool isLeft = simpleHands[i].isLeft;
                string handType = simpleHands[i].isLeft ? "Left" : "Right";     //pretty sweet way of knowing which hand

                
                for (int f=0; f<5; f++) {
                    int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
                    
                    ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
                    
                    fingersFound.push_back(id);
                    
                    if (isLeft) {
                        if (f == 0 || f == 1 || f == 2) {
                            
                            m.setAddress( "/" + handType);    //setup message
                            m.addStringArg("/fingers");
                            m.addStringArg("/" + fingerNames[f]);
                            m.addStringArg("/tip");
                            m.addFloatArg(tip.x);
                            m.addFloatArg(tip.y);
                            m.addFloatArg(tip.z);
                            oscSender.sendMessage( m );
                            m.clear();
                            
                            //to get the centroid of a 3d triangle
                            //((x1+x2+x3)/3,(y1+y2+y3)/3,(z1+z2+z3)/3)
                            centroidX += tip.x;   //add x,y,z coordinates of the 3 fingertips
                            centroidY += tip.y;
                            centroidZ += tip.z;
                        }
                    }
                    else{
                        if (f == 0 || f == 1) {
                            
                            m.setAddress( "/" + handType);    //setup message
                            m.addStringArg("/fingers");
                            m.addStringArg("/" + fingerNames[f]);
                            m.addStringArg("/tip");
                            m.addFloatArg(tip.x);
                            m.addFloatArg(tip.y);
                            m.addFloatArg(tip.z);
                            oscSender.sendMessage( m );
                            m.clear();
                        }
                    }
                }
                //divide centroid accumulators by 3 to get centroid's 3d coordinates
                centroidX = centroidX/3;
                centroidY = centroidY/3;
                centroidZ = centroidZ/3;
                
                //now send the centroid's coordinates over osc
                m.setAddress( "/centroid" );
                m.addFloatArg(centroidX);
                m.addFloatArg(centroidY);
                m.addFloatArg(centroidZ);
                oscSender.sendMessage( m );
                m.clear();
                
            }
        }

    }
    
    
    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
    leap.markFrameAsOld();
    
    //map the mouseX position to the relative position in the audio samples
    exis = ofMap(0, 0, ofGetWidth(), -(audio.getNumFrames()), audio.getNumFrames());
//    ye = ofMap(mouseY, 0, ofGetHeight(),1,-1);
    
    
    
}

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void ofApp::draw(){
    ofBackground(66,70,76);
    ofSetColor(50);
    ofDrawBitmapString("Leap Connected?: " + ofToString(leap.isConnected()), 20, 20);
    ofDrawBitmapString("hands found: " + ofToString(simpleHands.size()), 20, 40);
    ofDrawBitmapString("send all data?: " + ofToString(senderSwitch), 20, 60);

    cam.begin();
    
    //draw audio waveform
    ofPushMatrix();
    ofSetLineWidth(.003);
    //ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofScale((float) ofGetWidth() / audio.getNumFrames(), ofGetHeight() / 2);
    ofTranslate(-(audio.getNumFrames()/2), -0.13);
    ofSetColor(222, 222, 222);
    left.drawFaces();
    ofPopMatrix();
    
    fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    
    triangle.clearVertices();   //clear any previous vertices in the triangle Mesh

    float centroidX = 0.0, centroidY = 0.0, centroidZ = 0.0;  //accumulators for calculating centroid

    //draw hand
    for(int i = 0; i < simpleHands.size(); i++){
        
        bool isLeft        = simpleHands[i].isLeft;
        ofPoint handPos    = simpleHands[i].handPos;
        ofPoint handNormal = simpleHands[i].handNormal;
        
        for (int f=0; f < 5; f++) {
            ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip;  // fingertip
            
            //draw triangle
            ofPushMatrix();
            ofScale(3,3);
            if (isLeft) {
                if (f == 0 || f == 1 || f == 2) {
                    
                    //draw triangle
                    ofEnableAlphaBlending();
                    ofColor color(94,177,191,200);
                    float gray = ofMap(mouseX, 0, ofGetWidth(), 0, 255);
                    ofSetColor(64,124,172,200);
                    ofDrawSphere(tip.x, tip.y, tip.z, 4);
                    triangle.addVertex(tip);
                    triangle.addColor(color);
                    triangle.drawFaces();
                    ofSetLineWidth(.4);
                    triangle.drawWireframe();
                    ofDisableAlphaBlending();
                    
                    //to get the centroid of a 3d triangle
                    //((x1+x2+x3)/3,(y1+y2+y3)/3,(z1+z2+z3)/3)
                    centroidX += tip.x;   //add x,y,z coordinates of the 3 fingertips
                    centroidY += tip.y;
                    centroidZ += tip.z;
                    
                }
                
            }
            // get right hand index and thumb's X coord for selection area
            else{
                if(f==0){
                    leftSelection = ofMap(tip.x, -300, 300, 0, audio.getNumFrames());
                }
                else if (f==1){
                    rightSelection = ofMap(tip.x, -300, 300, 0, audio.getNumFrames());
                }
            }
            ofPopMatrix();
        }
        
        //draw selection area
        if(!isLeft){
            ofPushMatrix();
            ofSetLineWidth(1);
            ofScale((float) ofGetWidth() / audio.getNumFrames(), ofGetHeight() / 2);
            ofTranslate(exis,-1);
            ofEnableAlphaBlending();
            ofSetColor(227, 60, 54,185); // bien rojo
            ofRect(leftSelection, 0, rightSelection - leftSelection, ofGetHeight());
            ofDisableAlphaBlending();
            ofPopMatrix();
        }
    }
    
    // ugly hack to get the centroid only to draw when the left hand is present
    if((simpleHands.size() > 0 && simpleHands[0].isLeft) || (simpleHands.size() > 1 && simpleHands[1].isLeft)){
        
        //divide centroid accumulators by 3 to get centroid's 3d coordinates
        centroidX = centroidX/3;
        centroidY = centroidY/3;
        centroidZ = centroidZ/3;
        
        ofPushMatrix();
        ofEnableAlphaBlending();
        ofScale(3, 3);
        ofSetColor(4, 42, 43, 220);
        ofDrawSphere(centroidX, centroidY, centroidZ, 6);
        ofDisableAlphaBlending();
        ofPopMatrix();
        
    }
    
    cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == ' '){
        senderSwitch = !senderSwitch;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    ofxOscMessage m;
    m.setAddress( "/test" );
    m.addStringArg( "hands found" );
    oscSender.sendMessage( m );
    m.clear();
    
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
void ofApp::dragEvent(ofDragInfo dragInfo) {
    string soundFile = string(dragInfo.files[0]) ;
    loadFile(soundFile);
    soundFile = soundFile;
    
    ofxOscMessage m;
    m.setAddress("/soundFile");
    m.addStringArg( soundFile );
    oscSender.sendMessage(m);
    m.clear();
}

//-------------------------------------------------------------
void ofApp::loadFile(string filename) {
    audio.load(filename);

    left.clear();
    right.clear();

    left.setMode(OF_PRIMITIVE_LINE_STRIP);
    right.setMode(OF_PRIMITIVE_LINE_STRIP);

    const vector<float>& rawSamples = audio.getRawSamples();
    int channels = audio.getChannels();
    int n = rawSamples.size();
    for(int c = 0; c < channels; c++) {
        for(int i = c; i < n; i += channels) {
            (c == 0 ? left : right).addVertex(ofVec2f(i / channels, rawSamples[i]));
        }
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
