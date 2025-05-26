#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
	private:
		ofImage src;
		ofImage dst;
		ofImage nearDst;
		ofImage reductionDst;
		ofImage otsuDst;
		ofImage sepiaDst;
		ofImage grayDst;
		ofImage medianDst;

		int width, height;
		int dx;
		int expansion;
		int reduce;

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void chromaticAberration();
		void nearestNeighbor();
		void reduction();
		void thresholding();
		void mask();
		void sepia();
		void gray();
		void medianFilter();
		
};
