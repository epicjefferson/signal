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
    
    //here is a simple example of getting the hands and drawing each finger and joint
    //the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
    
    //if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method.
    //there you can work with the frame data directly.
    
    
    
    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms.
    
    
    simpleHands = leap.getSimpleHands();
//
//    if( leap.isFrameNew() && simpleHands.size() ){
//        
//        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
//        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
//        leap.setMappingZ(-150, 150, -200, 200);
//        
//        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
//        
//        for(int i = 0; i < simpleHands.size(); i++){
//            for (int f=0; f<5; f++) {
//                int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
//                ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
//                ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
//                ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
//                ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
//                fingersFound.push_back(id);
//            }
//        }
//    }
    
    
    
    //Option 2: Work with the leap data / sdk directly - gives you access to more properties than the simple approach
    //uncomment code below and comment the code above to use this approach. You can also inhereit ofxLeapMotion and get the data directly via the onFrame callback.
    
    //     vector <Hand> hands = leap.getLeapHands();
    //     if( leap.isFrameNew() && hands.size() ){
    //
    //         //leap returns data in mm - lets set a mapping to our world space.
    //         //you can get back a mapped point by using ofxLeapMotion::getMappedofPoint with the Leap::Vector that tipPosition returns
    //         leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
    //         leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
    //         leap.setMappingZ(-150, 150, -200, 200);
    //
    //         fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    //
    //         for(int i = 0; i < hands.size(); i++){
    //             for(int j = 0; j < 5; j++){
    //                 ofPoint pt;
    //
    //                 const Finger & finger = hands[i].fingers()[ fingerTypes[j] ];
    //
    //                 //here we convert the Leap point to an ofPoint - with mapping of coordinates
    //                 //if you just want the raw point - use ofxLeapMotion::getofPoint
    //                 pt = leap.getMappedofPoint( finger.tipPosition() );
    ////                 pt = leap.getMappedofPoint( finger.jointPosition(finger.JOINT_DIP) );
    //
    //                 fingersFound.push_back(finger.id());
    //             }
    //         }
    //     }
    
    if(senderSwitch){
        //send all leap data over OSC
        hands = leap.getLeapHands();
        
        if (hands.size() > 0) {
            for (int h = 0; h < hands.size(); h++){
                
                ofxOscMessage m;
    //              //test message
    //            m.setAddress( "/test" );
    //            m.addIntArg( h );
    //             m.addFloatArg( 3.5f );
    //            m.addStringArg( "hands found" );
    //             m.addFloatArg( ofGetElapsedTimef() );
    //            oscSender.sendMessage( m );
    //            m.clear();
                
                // Get the current hand
                Hand & hand = hands[h];
                
                string handType = hand.isLeft() ? "Left" : "Right";     //pretty sweet way of knowing which hand
                
                ////send handType over OSC
                // m.setAddress("/handType");
                // m.addStringArg(handType);
                // oscSender.sendMessage(m);
                // m.clear();
                
                //get palm position
                //cout << ", palm" << hand.palmPosition() << endl;
                
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
                
                // cout << string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
                //             << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
                //             << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << endl;
                
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
                
                // cout << string(2, ' ') <<  "Arm direction: " << arm.direction()
                //   << " wrist position: " << arm.wristPosition()
                //   << " elbow position: " << arm.elbowPosition() << endl;
                
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
                    
                    // cout << string(4, ' ') <<  fingerNames[finger.type()]
                    //         << " finger, id: " << finger.id()
                    //         << ", length: " << finger.length()
                    //         << "mm, width: " << finger.width() << endl;
                    
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
                        
                        // cout << string(6, ' ') <<  boneNames[boneType]
                        //           << " bone, start: " << bone.prevJoint()
                        //           << ", end: " << bone.nextJoint()
                        //           << ", direction: " << bone.direction() << endl;
                        
                        
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
                        
    //                    //sending current bones' next joint
    //                    m.setAddress("/" + handType);
    //                    m.addStringArg("/fingers");
    //                    m.addStringArg("/" + fingerNames[finger.type()]);
    ////                    m.addStringArg("/" + boneNames[boneType]);
    ////                    m.addStringArg("/nextJoint");
    //                    m.addFloatArg(finger.tipPosition()[1]) ;       // x
    //                    m.addFloatArg(bone.nextJoint()[1]);       // y
    //                    m.addFloatArg(bone.nextJoint()[2]);       // z
    //                    oscSender.sendMessage(m);
    //                    m.clear();
                    }
                    
                }
                
            }
        }
    }
    else{
        //if we're not sending out all leap data,
        //just use the triangle (left hand) + centroid
        //and selection (right hand)
        
//        simpleHands = leap.getSimpleHands();
        
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
//                    ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
//                    ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
//                    ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
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
                        
//                        simpleHands[i].armPos;
//                        
//                        m.setAddress("/" + handType);
//                        m.addStringArg("/wristPosition");
//                        m.addFloatArg(arm.wristPosition()[0]);       // x
//                        m.addFloatArg(arm.wristPosition()[1]);       // y
//                        m.addFloatArg(arm.wristPosition()[2]);       // z
//                        oscSender.sendMessage(m);
//                        m.clear();
                    }
                }
                //divide centroid accumulators by 3 to get centroid's 3d coordinates
                centroidX = centroidX/3;
                centroidY = centroidY/3;
                centroidZ = centroidZ/3;
                
                //now send the centroid's coordinates over osc
//                ofxOscMessage m;
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
//    ofBackgroundGradient(ofColor(169,221,214), ofColor(173,198,152), OF_GRADIENT_BAR);
//    ofBackground(15,29,45);   //azul processing
    ofBackground(66,70,76);
    ofSetColor(50);
    ofDrawBitmapString("Leap Connected?: " + ofToString(leap.isConnected()), 20, 20);
    ofDrawBitmapString("hands found: " + ofToString(simpleHands.size()), 20, 40);
    ofDrawBitmapString("send all data?: " + ofToString(senderSwitch), 20, 60);


    cam.begin();
    
//    //draw leap grid
//    ofPushMatrix();
//    ofRotate(90, 0, 0, 1);
//    ofSetColor(237,227,238);
//    ofDrawGridPlane(800, 20, false);
//    ofPopMatrix();
    
    //draw audio waveform
    ofPushMatrix();
    ofSetLineWidth(.003);
    //ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofScale((float) ofGetWidth() / audio.getNumFrames(), ofGetHeight() / 2);
    ofTranslate(-(audio.getNumFrames()/2), -0.13);
//    ofSetColor(192,87,70);
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
        
        //draw sphere in center of palm and line for palm direction
//        ofSetColor(0,0,230);
//        ofDrawSphere(handPos.x, handPos.y, handPos.z, 10);
//        ofSetColor(180,236,40);
//        ofDrawArrow(handPos, handPos + 50*handNormal);
        
        for (int f=0; f < 5; f++) {
            ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip;  // fingertip

//            //draw joints
//            ofSetColor(0);
//            ofDrawSphere(mcp.x, mcp.y, mcp.z, 7);
//            ofDrawSphere(pip.x, pip.y, pip.z, 7);
//            ofDrawSphere(dip.x, dip.y, dip.z, 7);
//            ofDrawSphere(tip.x, tip.y, tip.z, 7);
//
//            //drawbones
//            ofSetColor(0);
//            ofSetLineWidth(3);
//            ofLine(mcp.x, mcp.y, mcp.z, pip.x, pip.y, pip.z);
//            ofLine(pip.x, pip.y, pip.z, dip.x, dip.y, dip.z);
//            ofLine(dip.x, dip.y, dip.z, tip.x, tip.y, tip.z);
            
            
            
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
//            ofSetColor(89,127,66,100);
            ofSetColor(227, 60, 54,185); // bien rojo
//            ofSetColor(241, 101, 55);
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
        
//        //now send the centroid's coordinates over osc
//        ofxOscMessage m;
//        m.setAddress( "/centroid" );
//        m.addFloatArg(centroidX);
//        m.addFloatArg(centroidY);
//        m.addFloatArg(centroidZ);
//        oscSender.sendMessage( m );
//        m.clear();
        
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
