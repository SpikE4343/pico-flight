#include "ofMain.h"
// #include "ofxDatGui.h"
#include "ofxTelemetry.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY );
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onConnectButtonEvent(ofxDatGuiButtonEvent e);

    // ofxDatGui* gui;
    // ofxDatGui* varsGui;

    int port;
    int baud;
    bool connected;

    // ofxDatGuiDropdown* portGui;
    // ofxDatGuiDropdown* baudGui;
    // ofxDatGuiButton* connectGui;


    ofSerial*	serial;
    ofxTelemetry telemetry;

    uint tIndex;
    vector<ofxDatGuiTheme*> themes;

    ofTrueTypeFont font;
    ofEasyCam cam;

    ofParameter<float> prOffset;
    ofParameter<float> prLineHeight;
    ofParameter<float> prGraphStep;


    ofxNodeGuiCanvas* canvas;
    ofxNodeGui* guiLive;
    ofxNodeGui* guiConfig;
    ofxNodeGuiStyle style;

    bool inspect;
};