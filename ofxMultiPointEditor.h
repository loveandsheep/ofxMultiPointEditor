//
//  ofxMultiPointEditor.h
//  pointEditorExample
//
//  Created by Sheep on 12/08/15.
//  Copyright (c) 2012. All rights reserved.
//
#include "ofxXmlSettings.h"
#include "ofxClickDownMenu.h"
#include <deque>

#define MOUSE_STAT_DEFAULT 0
#define MOUSE_STAT_HOVER 1

#define PHASE_POINT 0
#define PHASE_RECT 1
#define PHASE_TRIANGLE 2

struct ofxMPERect{
	int idx[4];
};

struct ofxMPETriangle{
	int idx[3];
};

class ofxMultiPointEditor{
public:
	ofxMultiPointEditor();
	~ofxMultiPointEditor();
	
	void SetArea(ofRectangle rect);
	void SetArea(int x,int y,int w,int h);
	ofFbo buffer;
	ofRectangle drawArea;
	bool bAllocated;
	
	void mouseMoved(ofMouseEventArgs & args);
	void mousePressed(ofMouseEventArgs & args);
	void mouseReleased(ofMouseEventArgs & args);
	void mouseDragged(ofMouseEventArgs & args);
	void keyPressed(ofKeyEventArgs & key);
	void keyReleased(ofKeyEventArgs & key);
	
	void cdmEvent(ofxCDMEvent &ev);
	void draw();
	
	void load(string fname);
	void save(string fname);
	
	void setChild(ofxMultiPointEditor * child);
	void sync_Pts();
	void sync_Rects();
	void sync_Tris();
	bool hasChild;
	bool isChild;
	
	int active_pt;
	int Edit_phase;
	int rect_add_phase;
	int tri_add_phase;
	int temp_pts[4];
	
	deque<ofPoint> pts;
	deque<ofxMPERect> rects;
	deque<ofxMPETriangle> tris;
	vector<ofxMultiPointEditor*> children;
	
	int menu_id;
	ofxClickDownMenu menu;
	bool bSnap;
	bool Snapping_h;
	bool Snapping_v;
};