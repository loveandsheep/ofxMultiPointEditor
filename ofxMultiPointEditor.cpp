//
//  ofxMultiPointEditor.cpp
//
//  Created by Sheep on 12/08/15.
//  Copyright (c) 2012. All rights reserved.
//

#include "ofxMultiPointEditor.h"

ofxMultiPointEditor::ofxMultiPointEditor(){
	bAllocated = false;
	
	active_pt = -1;
	vector<string> boolBranch;
	boolBranch.push_back("ON");
	boolBranch.push_back("OFF");
	
	vector<string> PhaseBranch;
	PhaseBranch.push_back("POINT");
	PhaseBranch.push_back("RECT");
	PhaseBranch.push_back("TRIANGLE");
	
	menu_id = ofRandom(100000);
	menu.menu_name = "pts_Menu"+ofToString(menu_id);
	menu.OnlyRightClick = true;
	menu.RegisterMenu("Delete");
	menu.RegisterBranch("Snap", &boolBranch);
	menu.RegisterBranch("Make", &PhaseBranch);
	
	
	
	ofAddListener(ofxCDMEvent::MenuPressed, this, &ofxMultiPointEditor::cdmEvent);
	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);
	
	bSnap = true;
	Edit_phase = PHASE_POINT;
}

ofxMultiPointEditor::~ofxMultiPointEditor(){
	ofUnregisterKeyEvents(this);
	ofUnregisterKeyEvents(this);
}

void ofxMultiPointEditor::draw(){
	buffer.begin();
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int i = 0;i < pts.size();i++){
		ofSetRectMode(OF_RECTMODE_CENTER);
		ofSetColor(255, 0, 187);
		if (active_pt == i){
			ofCircle(pts[i].x, pts[i].y, 5);
		}

		ofNoFill();
		ofSetHexColor(0xFFFFFF);
		ofLine(pts[i].x-6, pts[i].y, pts[i].x+6, pts[i].y);
		ofLine(pts[i].x, pts[i].y-6, pts[i].x, pts[i].y+6);
		ofFill();
		
		ofSetRectMode(OF_RECTMODE_CORNER);
	}
	for (int i = 0;i < rects.size();i++){
		ofSetHexColor(0xFFFFFF);
		glBegin(GL_LINE_LOOP);
		for (int j = 0;j < 4;j++){
			glVertex2f(pts[rects[i].idx[j]].x, pts[rects[i].idx[j]].y);
		}
		glEnd();
	}
	for (int i = 0;i < tris.size();i++){
		ofSetHexColor(0xFFFFFF);
		glBegin(GL_LINE_LOOP);
		for (int j = 0;j < 3;j++){
			glVertex2f(pts[tris[i].idx[j]].x, pts[tris[i].idx[j]].y);
		}
		glEnd();
	}
	
	//特別な状態遷移の時
	if ((bSnap)&&(ofGetMousePressed())&&(active_pt != -1)){
		ofSetColor(255,200);
		if (Snapping_v) ofLine(pts[active_pt].x,0,pts[active_pt].x,drawArea.height);
		if (Snapping_h )ofLine(0, pts[active_pt].y, drawArea.width, pts[active_pt].y);
	}
	if ((Edit_phase == PHASE_RECT)||(Edit_phase == PHASE_TRIANGLE)){
		glBegin(GL_LINE_STRIP);
		for (int i = 0;i < ((Edit_phase == PHASE_RECT) ? rect_add_phase : tri_add_phase);i++){
			glVertex2f(pts[temp_pts[i]].x, pts[temp_pts[i]].y);
		}
		glVertex2f((ofGetMouseX() - drawArea.x)*buffer.getWidth()/drawArea.width,
				   (ofGetMouseY() - drawArea.y)*buffer.getHeight()/drawArea.height);
		glEnd();
	}
	
	buffer.end();
	
	ofSetHexColor(0xFFFFFF);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	buffer.draw(drawArea.x,drawArea.y,drawArea.width,drawArea.height);
	menu.draw();
	
	if (menu.phase == PHASE_WAIT){
		string mouseInfo = "";
		if (Edit_phase == PHASE_POINT){
			mouseInfo += "Point::";
			if (active_pt == -1) mouseInfo += "Make Pt.";
			if (active_pt != -1) mouseInfo += "Drag&move Pt.";
		}else if (Edit_phase == PHASE_RECT){
			mouseInfo += "Rect::";
			mouseInfo += "Select " + ofToString(rect_add_phase) + " Pt.";
		}else if (Edit_phase == PHASE_TRIANGLE){
			mouseInfo += "Triangle::";
			mouseInfo += "Select " + ofToString(tri_add_phase) + " Pt.";
		}
		ofSetColor(0, 0, 0,200);
		ofRect(ofGetMouseX()+8, ofGetMouseY()+20, mouseInfo.length()*8, 14);
		ofSetHexColor(0xFFFFFF);
		ofDrawBitmapString(mouseInfo, ofGetMouseX()+10,ofGetMouseY()+30);		
	}
}

void ofxMultiPointEditor::SetArea(ofRectangle rect){
	SetArea(rect.x,rect.y,rect.width,rect.height);
}

void ofxMultiPointEditor::SetArea(int x, int y, int w, int h){
	if (!bAllocated){
		buffer.allocate(w, h);
		bAllocated = true;
	}else{
		
	}
	
	drawArea = ofRectangle(x,y,w,h);
}

void ofxMultiPointEditor::cdmEvent(ofxCDMEvent &ev){
	string menid = "pts_Menu" + ofToString(menu_id) + "::";
	if ((ev.message == menid + "Delete")&&(active_pt != -1)){
		int cnt = 0;
		while (cnt < rects.size()){//四角形の後処理
			bool Checker = false;
			for (int i = 0;i < 4;i++){
				if (rects[cnt].idx[i] == active_pt){
					Checker = true;
				}
			}
			if (Checker){
				rects.erase(rects.begin() + cnt);
			}else{
				cnt++;
			}
		}cnt = 0;
		while (cnt < tris.size()){//三角形の後処理
			bool Checker = false;
			for (int i = 0;i < 4;i++){
				if (tris[cnt].idx[i] == active_pt){
					Checker = true;
				}
			}
			if (Checker){
				tris.erase(tris.begin() + cnt);
			}else{
				cnt++;
			}
		}cnt = 0;
		
		for (int i = 0;i < rects.size();i++){
			for (int j = 0;j < 4;j++){
				if (rects[i].idx[j] > active_pt){
					rects[i].idx[j]--;
				}
			}
		}
		for (int i = 0;i < tris.size();i++){
			for (int j = 0;j < 3;j++){
				if (tris[i].idx[j] > active_pt){
					tris[i].idx[j]--;
				}
			}			
		}
		
		pts.erase(pts.begin() + active_pt);
	}
	if (ev.message == menid + "Snap::ON" ) bSnap = true;
	if (ev.message == menid + "Snap::OFF") bSnap = false;
	
	if (ev.message == menid + "Make::RECT") {
		Edit_phase = PHASE_RECT;
		rect_add_phase = 0;
	}
	if (ev.message == menid + "Make::TRIANGLE"){
		Edit_phase = PHASE_TRIANGLE;
		tri_add_phase = 0;
	}
	if (ev.message == menid + "Make::POINT"){
		Edit_phase = PHASE_POINT;
	}
	
}

void ofxMultiPointEditor::mouseMoved(ofMouseEventArgs & args){
	menu.Enable = ((drawArea.x < args.x)&&(args.x < drawArea.x+drawArea.width)&&
				   (drawArea.y < args.y)&&(args.y < drawArea.y+drawArea.height));
	if (menu.phase == PHASE_WAIT){
		active_pt = -1;
		for (int i = 0;i < pts.size();i++){
			if (pts[i].distance(ofVec3f((args.x - drawArea.x)*buffer.getWidth()/drawArea.width,
										(args.y - drawArea.y)*buffer.getHeight()/drawArea.height,0)) < 13){
				active_pt = i;
			}
		}
	}
		
}
void ofxMultiPointEditor::mousePressed(ofMouseEventArgs & args){
	if (Edit_phase == PHASE_POINT){
		if ((menu.phase == PHASE_WAIT)&&(active_pt == -1)) {//新規ポイントの作成
			pts.push_back(ofPoint((args.x - drawArea.x)*buffer.getWidth()/drawArea.width,(args.y - drawArea.y)*buffer.getHeight()/drawArea.height));
			active_pt = pts.size() - 1;
		}
	}else if (Edit_phase == PHASE_RECT){
		if ((menu.phase == PHASE_WAIT)&&(active_pt != -1)){//新規四角形の作成
			temp_pts[rect_add_phase] = active_pt;
			rect_add_phase++;
			if (rect_add_phase == 4){
				rect_add_phase = 0;
				ofxMPERect r;
				for (int i = 0;i < 4;i++){
					r.idx[i] = temp_pts[i];
				}
				rects.push_back(r);
			}
		}
	}else if (Edit_phase == PHASE_TRIANGLE){
		if ((menu.phase == PHASE_WAIT)&&(active_pt != -1)){
			temp_pts[tri_add_phase] = active_pt;
			tri_add_phase++;
			if (tri_add_phase == 3){
				tri_add_phase = 0;
				ofxMPETriangle t;
				for (int i = 0;i < 3;i++){
					t.idx[i] = temp_pts[i];
				}
				tris.push_back(t);
			}
		}
	}
}
void ofxMultiPointEditor::mouseReleased(ofMouseEventArgs & args){
	
}
void ofxMultiPointEditor::mouseDragged(ofMouseEventArgs & args){
	if ((active_pt != -1)&&(Edit_phase == PHASE_POINT)){
		pts[active_pt] = ofPoint(ofPoint(MAX(0,MIN(drawArea.width,(args.x - drawArea.x)*buffer.getWidth()/drawArea.width)),
										 MAX(0,MIN(drawArea.height,(args.y - drawArea.y)*buffer.getHeight()/drawArea.height))));
		//Snap
		if (bSnap){
			Snapping_h = false;
			Snapping_v = false;
			for (int i = 0;i < pts.size();i++){
				if ((i != active_pt)&&(abs(pts[active_pt].x - pts[i].x) < 5)){
					pts[active_pt].x = pts[i].x;
					Snapping_v = true;
				}
				if ((i != active_pt)&&(abs(pts[active_pt].y - pts[i].y) < 5)){
					pts[active_pt].y = pts[i].y;
					Snapping_h = true;
				}
			}
		}
	}
}

void ofxMultiPointEditor::keyPressed(ofKeyEventArgs & key){
	
}
void ofxMultiPointEditor::keyReleased(ofKeyEventArgs & key){
	
}

