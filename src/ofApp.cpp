#include "ofApp.h"
#include "ofxMeshUtils.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    ofSetCircleResolution(80);
    ofBackground(54, 54, 54);
    
    // 0 output channels,
    // 2 input channels
    // 44100 samples per second
    // 256 samples per buffer
    // 4 num buffers (latency)
    
    soundStream.listDevices();
    
    //if you want to set a different device id
    //soundStream.setDeviceID(0); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
    
    int bufferSize = 256;
    
    
    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    volHistory.assign(400, 0.0);
    
    bufferCounter	= 0;
    drawCounter		= 0;
    smoothedVol     = 0.0;
    scaledVol		= 0.0;
    
    soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
    
    
    
    
    ////////////mesh
    
    
    gui.setup("Superformula");
    gui.add(a1value.setup("a1value", 1,0,5));
    gui.add(a2value.setup("a2value", 1,0,5));
    gui.add(n1value.setup("n1value", 8,0,40));
    gui.add(n2value.setup("n2value", 1,0,5));
    gui.add(n3value.setup("n3value", 1,0,5));
    gui.add(n4value.setup("n4value", 1,0,5));
    gui.add(stepvalue.setup("stepvalue", 0.05,0.02,0.9));
    gui.add(drawWire.setup("Draw Wireframe", false));
    gui.add(drawPoints.setup("Draw Points", false));
    
    //gui.loadFromFile("Superformula.xml");
    
    guiRect = ofRectangle(gui.getPosition().x, gui.getPosition().y, gui.getWidth(), gui.getHeight());
    
    
    light.setDirectional();
    light.setOrientation(ofVec3f(30,60,60));
    
    a1target = a1value;
    a2target = a2value;
    n1target = n1value;
    n2target = n2value;
    n3target = n3value;
    n4target = n4value;
    
    showGui = false;
    
    
}


//////mesh


float moveTowards(float current, float target, float step) {
    
    if(abs(current-target) < step) {
        current = target;
    } else {
        
        if(current > target) {
            current -= step;
        } else if(current < target) {
            current += step;
        }
    }
    
    return current;
}


ofVec3f ofApp::sf3d(float x, float y) {
    
    float i = -PI + x*stepvalue;
    
    float j = -PI/2.0 + y*stepvalue;
    
    float raux1 = pow(abs(1/a1value*abs(cos(n1value * i/4))),n3value)+pow(abs(1/a2value*abs(sin(n1value*i/4))),n4value);
    
    float r1=pow(abs(raux1),(-1/n2value));
    float raux2=pow(abs(1/a1value*abs(cos(n1value*j/4))),n3value)+pow(abs(1/a2value*abs(sin(n1value*j/4))),n4value);
    float r2=pow(abs(raux2),(-1/n2value));
    float xx=r1*cos(i)*r2*cos(j)*100.0f;
    float yy=r1*sin(i)*r2*cos(j)*100.0f;
    float zz=r2*sin(j)*100.0f;
    
    return ofVec3f(xx,yy,zz);
}

//--------------------------------------------------------------
void ofApp::update(){
    //lets scale the vol up to a 0-1 range
    scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
    
    //lets record the volume into an array
    volHistory.push_back( scaledVol );
    
    //if we are bigger the the size we want to record - lets drop the oldest value
    if( volHistory.size() >= 400 ){
        volHistory.erase(volHistory.begin(), volHistory.begin()+1);
    }
    
    //////mesh
    
    
    
    
    
    a1value = moveTowards(a1value, a1target, 0.02);
    a2value = moveTowards(a2value, a2target, 0.02);
    n1value = moveTowards(n1value, n1target, 0.02);
    n2value = moveTowards(n2value, n2target, 0.02);
    n3value = moveTowards(n3value, n3target, 0.02);
    n4value = moveTowards(n4value, n4target, 0.02);
    
    
    mesh.clear();
    int N_X = ceil((2.0*PI) / stepvalue);
    int N_Y = ceil(PI / stepvalue);
    
    for(int x=0;x<N_X;x++) {
        for(int y=0;y<N_Y;y++) {
            mesh.addVertex(sf3d(x,y));
            mesh.addColor(ofColor(255,255,255,100));
        }
    }
    
    mesh.addVertex(sf3d(PI/stepvalue,PI/stepvalue));
    mesh.addColor(ofColor(scaledVol *4000));
    
    
    lastRow.clear();
    
    for(int x=0;x<N_X;x++) {
        for(int y=0;y<N_Y-1;y++) {
            
            if(x == N_X-1) {
                
                int idx1 = x*N_Y+y;
                int idx2 = x*N_Y+y+1;
                
                int idx3 = y+1;
                int idx4 = y;
                
                mesh.addTriangle(idx1, idx2, idx3);
                mesh.addTriangle(idx1, idx3, idx4);
                
            } else {
                int idx1 = x*N_Y+y;
                int idx2 = x*N_Y+y+1;
                
                int idx3 = (x+1)*N_Y+y+1;
                int idx4 = (x+1)*N_Y+y;
                
                mesh.addTriangle(idx1, idx2, idx3);
                mesh.addTriangle(idx1, idx3, idx4);
                
                if(y == N_Y-2) {
                    lastRow.push_back(idx2);
                }
            }
            
        }
    }
    
    int lastVertex = mesh.getNumVertices()-1;
    
    for(int i=0;i<lastRow.size()-1;i++) {
        mesh.addTriangle(lastRow[i], lastRow[i+1], lastVertex);
    }
    mesh.addTriangle(lastRow[0],lastRow[lastRow.size()-1], lastVertex);
    
    ofxMeshUtils::calcNormals(mesh);
    
    
    

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    
    
    
    ofBackground(0);
    glShadeModel(GL_FLAT);
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    
    cam.begin();
    ofPushMatrix();
    
    
    if(!drawPoints) {
        ofDisableAlphaBlending();
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        
        glEnable(GL_DEPTH_TEST);
        ofEnableLighting();light.enable();
        
        mesh.clearColors();
        ofSetColor(scaledVol *4000);
       
        mesh.draw();
        light.disable();ofDisableLighting();
    } else {
        mesh.setMode(OF_PRIMITIVE_POINTS);
        
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_PROGRAM_POINT_SIZE_ARB);
        
        glPointSize(0.9f);
        mesh.clearColors();
        ofSetColor(scaledVol *4000);
        ofEnableAlphaBlending();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        
        mesh.draw();
    }
    
    
    
    if(drawWire) {
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        mesh.clearColors();
        ofSetColor(scaledVol *2000);
        mesh.drawWireframe();
    }
    
    
    glDisable(GL_DEPTH_TEST);
    
    cam.end();
    
    if(showGui)
        gui.draw();
    
    
////////////audio
    
    ofSetColor(225);
    ofDrawBitmapString(" PAC INTERACTIVE STUDIO", 32, 32);
    
    
    ofNoFill();
    
    // draw the left channel:
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(32, 170, 0);
    
    ofSetColor(225);
    ofDrawBitmapString("Left Channel", 4, 18);
    
    ofSetLineWidth(1);
    ofRect(0, 0, 512, 200);
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    
    ofBeginShape();
    for (unsigned int i = 0; i < left.size(); i++){
        ofVertex(i*2, 100 -left[i]*180.0f);
    }
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    /*
    // draw the right channel:
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(32, 370, 0);
    
    ofSetColor(225);
    ofDrawBitmapString("Right Channel", 4, 18);
    
    ofSetLineWidth(1);
    ofRect(0, 0, 512, 200);
    
    ofSetColor(245, 58, 135);
    ofSetLineWidth(3);
    
    ofBeginShape();
    for (unsigned int i = 0; i < right.size(); i++){
        ofVertex(i*2, 100 -right[i]*180.0f);
    }
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    
    // draw the average volume:
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(565, 170, 0);
    
    ofSetColor(225);
    ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(scaledVol * 100.0, 0), 4, 18);
    ofRect(0, 0, 400, 400);
    
    ofSetColor(245, 58, 135);
    ofFill();
    ofCircle(200, 200, scaledVol * 190.0f);
    
    //lets draw the volume history as a graph
    ofBeginShape();
    for (unsigned int i = 0; i < volHistory.size(); i++){
        if( i == 0 ) ofVertex(i, 400);
        
        ofVertex(i, 400 - volHistory[i] * 70);
        
        if( i == volHistory.size() -1 ) ofVertex(i, 400);
    }
    ofEndShape(false);
    
    ofPopMatrix();
    ofPopStyle();
    
    drawCounter++;
    
    ofSetColor(225);
    string reportString = "buffers received: "+ofToString(bufferCounter)+"\ndraw routines called: "+ofToString(drawCounter)+"\nticks: " + ofToString(soundStream.getTickCount());
    ofDrawBitmapString(reportString, 32, 589);
    
    */
    
    
    /////esta es la entarda de audio
    cout << scaledVol;
    
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    
    float curVol = 0.0;
    
    // samples are "interleaved"
    int numCounted = 0;
    
    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        left[i]		= input[i*2]*0.5;
        right[i]	= input[i*2+1]*0.5;
        
        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted+=2;
    }
    
    //this is how we get the mean of rms :)
    curVol /= (float)numCounted;
    
    // this is how we get the root of rms :)
    curVol = sqrt( curVol );
    
    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;
    
    bufferCounter++;
    
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    if(key == 'r') {
        a1target = ofRandomuf()*3.0f+0.5f;
        a2target = ofRandomuf()*3.0f+0.5f;
        n1target = ofRandomuf()*20.0f;
        n2target = ofRandomuf()*3.0f+0.5f;
        n3target = ofRandomuf()*3.0f+0.5f;
        n4target = ofRandomuf()*3.0f+0.5f;
    }
    
    if(key == 'p') {
        drawPoints = !drawPoints;
    }
    
    if(key == 'w') {
        drawWire = !drawWire;
    }
    
    if(key == 'g') {
        showGui = !showGui;
    }
    
    if(key == '1') {
        a1value = 0.5;
    }
    if(key == '2') {
        a2value = 0.5;
    }
    if(key == '3') {
        n1value = 0.5;
    }
    if(key == '4') {
        n2value = 0.5;
    }
    
    if(key == '5') {
        n3value = 0.5;
    }
    
    if(key == '6') {
        n4value = 0.5;
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

