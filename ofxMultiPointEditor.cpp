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

	vector<string> PresetBranch;
	PresetBranch.push_back("Preset0");
	PresetBranch.push_back("Preset1");
	PresetBranch.push_back("Preset2");
	PresetBranch.push_back("Preset3");
	PresetBranch.push_back("Preset4");
	PresetBranch.push_back("Preset5");
	PresetBranch.push_back("Preset6");
	PresetBranch.push_back("Preset7");
	PresetBranch.push_back("Preset8");
	PresetBranch.push_back("Preset9");
	ofSeedRandom();
	menu_id = (int)ofRandom(100000);
	menu.menu_name = "pts_Menu"+ofToString(menu_id);
	menu.OnlyRightClick = true;
	menu.RegisterMenu("Delete");
	menu.RegisterBranch("Snap", &boolBranch);
	menu.RegisterBranch("Make", &PhaseBranch);
	menu.RegisterBranch("Load", &PresetBranch);
	menu.RegisterBranch("Save", &PresetBranch);	
	
	ofAddListener(ofxCDMEvent::MenuPressed, this, &ofxMultiPointEditor::cdmEvent);
	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);
	
	hasChild = false;
	isChild = false;
	bSnap = true;
	Edit_phase = PHASE_POINT;
	
	dialog.addNewMessage("Notice", "Not saved. Continue?", OFX_MESSAGEBOX_YESNO);
	notSaved = false;
	last_selected = -1;
}

ofxMultiPointEditor::~ofxMultiPointEditor(){
	ofUnregisterMouseEvents(this);
	ofUnregisterKeyEvents(this);
}

void ofxMultiPointEditor::draw(){
	if (active_pt != -1) last_selected = active_pt;
	if (dialog.getResponse() == 0){
		load(StayLoader);
	}
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
		if (i == last_selected){
			ofCircle(pts[i], 10);
		}
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
			if ((active_pt == -1)&&(!isChild)) mouseInfo += "Make Pt.";
			if (active_pt != -1) mouseInfo += "Drag&move Pt.";
		}else if (Edit_phase == PHASE_RECT){
			mouseInfo += "Rect::";
			mouseInfo += "Select " + ofToString(rect_add_phase) + " Pt.";
		}else if (Edit_phase == PHASE_TRIANGLE){
			mouseInfo += "Triangle::";
			mouseInfo += "Select " + ofToString(tri_add_phase) + " Pt.";
		}
		
		if (isChild) mouseInfo = "Child can move Pts only.";
		
		if ((drawArea.x < ofGetMouseX())&&(ofGetMouseX() < drawArea.x+drawArea.width)&&
			(drawArea.y < ofGetMouseY())&&(ofGetMouseY() < drawArea.y+drawArea.height)){
			ofSetColor(0, 0, 0,200);
			ofRect(ofGetMouseX()+8, ofGetMouseY()+20, mouseInfo.length()*8, 14);
			ofSetHexColor(0xFFFFFF);
			ofDrawBitmapString(mouseInfo, ofGetMouseX()+10,ofGetMouseY()+30);			
		}
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
		sync_Pts(active_pt);
		sync_Rects();
		sync_Tris();
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
	if ((ev.message.substr(0,menid.length()+4) == menid + "Save")&&
		(ev.message.substr(menid.length()+6) != "mouseFix")&&
		(ev.message.substr(menid.length()+6) != " X Cancel")){
		save(ev.message.substr(menid.length()+6));
	}
	if ((ev.message.substr(0,menid.length()+4) == menid + "Load")&&
		(ev.message.substr(menid.length()+6) != "mouseFix")&&
		(ev.message.substr(menid.length()+6) != " X Cancel")){
		if (!notSaved){
			load(ev.message.substr(menid.length()+6));
		}else{
			StayLoader = ev.message.substr(menid.length()+6);
			dialog.viewMessage(0);
		}
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
		if ((menu.phase == PHASE_WAIT)&&(active_pt == -1)&&
			(drawArea.x < args.x)&&(args.x < drawArea.x+drawArea.width)&&
			(drawArea.y < args.y)&&(args.y < drawArea.y+drawArea.height)&&
			(!isChild)) {//新規ポイントの作成
			pts.push_back(ofPoint(MAX(0,MIN(buffer.getWidth(),(args.x - drawArea.x)*buffer.getWidth()/drawArea.width)),
								  MAX(0,MIN(buffer.getHeight(),(args.y - drawArea.y)*buffer.getHeight()/drawArea.height))));
			active_pt = pts.size() - 1;
			sync_Pts(-1);
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
				sync_Rects();
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
				sync_Tris();
			}
		}
	}
}
void ofxMultiPointEditor::mouseReleased(ofMouseEventArgs & args){
	
}
void ofxMultiPointEditor::mouseDragged(ofMouseEventArgs & args){
	if ((active_pt != -1)&&(Edit_phase == PHASE_POINT)){
		notSaved = true;
		pts[active_pt] = ofPoint(MAX(0,MIN(buffer.getWidth(),(args.x - drawArea.x)*buffer.getWidth()/drawArea.width)),
										 MAX(0,MIN(buffer.getHeight(),(args.y - drawArea.y)*buffer.getHeight()/drawArea.height)));
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
	if (last_selected != -1){
		if (key.key == OF_KEY_UP)	pts[last_selected].y--;
		if (key.key == OF_KEY_DOWN)	pts[last_selected].y++;
		if (key.key == OF_KEY_LEFT) pts[last_selected].x--;
		if (key.key == OF_KEY_RIGHT)pts[last_selected].x++;
	}
}
void ofxMultiPointEditor::keyReleased(ofKeyEventArgs & key){
	
}

void ofxMultiPointEditor::load(string fname){
	ofxXmlSettings xml;
	fname += ".xml";
	xml.loadFile(fname);
	pts.clear();
	rects.clear();
	tris.clear();

	for (int i = 0;i < xml.getNumTags("PT");i++){
		ofPoint p;
		xml.pushTag("PT",i);
		p.x = xml.getValue("X", 0);
		p.y = xml.getValue("Y", 0);
		xml.popTag();
		pts.push_back(p);
	}
	
	for (int i = 0;i < xml.getNumTags("RECT");i++){
		ofxMPERect r;
		xml.pushTag("RECT",i);
		for (int j = 0;j < 4;j++){
			r.idx[j] = xml.getValue("IDX",0, j);
		}
		xml.popTag();
		rects.push_back(r);
	}
	
	for (int i = 0;i < xml.getNumTags("TRI");i++){
		ofxMPETriangle r;
		xml.pushTag("TRI",i);
		for (int j = 0;j < 3;j++){
			r.idx[j] = xml.getValue("IDX",0, j);
		}
		xml.popTag();
		tris.push_back(r);
	}
	for (int i = 0;i < children.size();i++){
		children[i]->load("child_"+ofToString(i)+"_"+fname);
	}
	notSaved = false;
}

void ofxMultiPointEditor::save(string fname){
	ofxXmlSettings xml;
	fname += ".xml";
	
	for (int i = 0;i < pts.size();i++){
		int tagNum = xml.addTag("PT");
		xml.setValue("PT:X",pts[i].x, tagNum);
		xml.setValue("PT:Y",pts[i].y, tagNum);
	}
	
	for (int i = 0;i < rects.size();i++){
		int tagNum = xml.addTag("RECT");
		xml.pushTag("RECT",tagNum);
		for (int j = 0;j < 4;j++){
			int idxNum = xml.addTag("IDX");
			xml.setValue("IDX",rects[i].idx[j], idxNum);
		}
		xml.popTag();
	}
	for (int i = 0;i < tris.size();i++){
		int tagNum = xml.addTag("TRI");
		xml.pushTag("TRI",tagNum);
		for (int j = 0;j < 3;j++){
			int idxNum = xml.addTag("IDX");
			xml.setValue("IDX",tris[i].idx[j], idxNum);
		}
		xml.popTag();
	}
	
	xml.saveFile(fname);
	for (int i = 0;i < children.size();i++){
		children[i]->save("child_"+ofToString(i)+"_"+fname);
	}
	notSaved = false;
	/*--------------------
	 <PT>
	 <X>xxx</X>
	 <Y>yyy</Y>
	 </PT>
	 
	 ...
	 
	 <RECT>
	 <IDX>1</IDX>
	 <IDX>2</IDX>
	 <IDX>3</IDX>
	 <IDX>4</IDX>
	 </RECT>
	 
	 ...
	 
	 <TRI>
	 <IDX>1</IDX>
	 <IDX>2</IDX>
	 <IDX>3</IDX>
	 </TRI>
	 
	 ...
	 ---------------------*/
}

void ofxMultiPointEditor::setChild(ofxMultiPointEditor *child){
	children.push_back(child);
	child->isChild = true;
	hasChild = true;
	child->menu.UnRegisterMenu("Delete");
	child->menu.UnRegisterMenu("Make");
	child->menu.UnRegisterMenu("Load");
	child->menu.UnRegisterMenu("Save");
}

void ofxMultiPointEditor::sync_Pts(int bMake){
	if (bMake == -1){
		for (int i = 0;i < children.size();i++){
			ofPoint p = pts[pts.size()-1];
			children[i]->pts.push_back(p);
		}
	}else{
		for (int i = 0;i < children.size();i++){
			children[i]->pts.erase(children[i]->pts.begin()+bMake);
		}
	}
	notSaved = true;
}

void ofxMultiPointEditor::sync_Rects(){
	notSaved = true;
	for (int i = 0;i < children.size();i++){
		children[i]->rects.clear();
		for (int j = 0;j < rects.size();j++){
			ofxMPERect p = rects[j];
			children[i]->rects.push_back(p);
		}
	}
	
}

void ofxMultiPointEditor::sync_Tris(){
	notSaved = true;
	for (int i = 0;i < children.size();i++){
		children[i]->tris.clear();
		for (int j = 0;j < tris.size();j++){
			ofxMPETriangle p = tris[j];
			children[i]->tris.push_back(p);
		}
	}
}
