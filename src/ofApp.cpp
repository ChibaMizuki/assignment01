#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//ゲキヤクアイコン
	//src.load("gkyk.jpg");

	// カードイラスト
	//src.load("mzk.jpg");

	//起き上がり小法師
	src.load("kbs.jpg");

	//POPSONG
	//src.load("yk.jpg");

	width = src.getWidth();
	height = src.getHeight();

	//色ずれの大きさ
	dx = 10;
	dst.allocate(width, height, OF_IMAGE_COLOR);

	//拡大率（整数値）
	expansion = 3;
	nearDst.allocate(width * expansion, height * expansion, OF_IMAGE_COLOR);

	//縮小率（1 / reduce, 整数値）
	reduce = 2.5;
	reductionDst.allocate(width / reduce, height / reduce, OF_IMAGE_COLOR);

	//セピア
	sepiaDst.allocate(width, height, OF_IMAGE_COLOR);

	//大津の二値化
	otsuDst.allocate(width, height, OF_IMAGE_GRAYSCALE);

	//グレースケール変換
	grayDst.allocate(width, height, OF_IMAGE_COLOR);

	//メディアンフィルター
	medianDst.allocate(width, height, OF_IMAGE_COLOR);

	//ピクセルソート
	pixelDst.allocate(width, height, OF_IMAGE_COLOR);

	//色ずれ
	chromaticAberration();
	//拡大
	nearestNeighbor();
	//縮小
	reduction();
	//セピア調フィルター
	sepia();
	//大津の二値化
	thresholding();
	//グレースケール変換
	gray();
	//medianフィルター
	medianFilter();
	//ピクセルソート
	pixelSort();
	convert();

	ofSetWindowShape(1500, 1500);

}

//--------------------------------------------------------------
void ofApp::chromaticAberration() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = dst.getPixels().getData();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = y * width + x;

			int R = src_data[index * 3 + 0];
			int G = src_data[index * 3 + 1];
			int B = src_data[index * 3 + 2];

			dst_data[index * 3 + 0] = R;
			dst_data[index * 3 + 1] = G;
			dst_data[index * 3 + 2] = B;

			//赤を左にずらす
			if (x + dx <= width) {
				R = src_data[(index * 3 + 0) + 3 * dx];
			}
			else {
				R = 0;
			}

			//青を右にずらす
			if (x - dx >= 0) {
				B = src_data[index * 3 + 2 - 3 * dx];
			}
			else {
				B = 0;
			}

			dst_data[index * 3 + 0] = R;
			dst_data[index * 3 + 1] = G;
			dst_data[index * 3 + 2] = B;
		}
	}
	dst.update();
	//dst.save("color.jpg");
}

//--------------------------------------------------------------
void ofApp::nearestNeighbor() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = nearDst.getPixels().getData();

	int expandedWidth = width * expansion;
	int expandedHeight = height * expansion;

	for (int y = 0; y < expandedHeight; y++) {
		for (int x = 0; x < expandedWidth; x++) {
			int dst_index = y * expandedWidth + x;
			int src_index = y / expansion * width + x / expansion;

			dst_data[dst_index * 3 + 0] = src_data[src_index * 3 + 0];
			dst_data[dst_index * 3 + 1] = src_data[src_index * 3 + 1];
			dst_data[dst_index * 3 + 2] = src_data[src_index * 3 + 2];
		}
	}
	nearDst.update();
	//nearDst.save("large.jpg");
}

//--------------------------------------------------------------
void ofApp::reduction() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = reductionDst.getPixels().getData();

	int reducedWidth = width / reduce;
	int reducedHeight = height / reduce;

	for (int y = 0; y < reducedHeight; y++) {
		for (int x = 0; x < reducedWidth; x++) {
			int dst_index = y * reducedWidth + x;
			int R = 0;
			int G = 0;
			int B = 0;

			for (int src_y = 0; src_y < reduce; src_y++) {
				for (int src_x = 0; src_x < reduce; src_x++) {
					int src_index = ((y * reduce + src_y)* width) + (x * reduce + src_x);

					R += src_data[src_index * 3 + 0];
					G += src_data[src_index * 3 + 1];
					B += src_data[src_index * 3 + 2];
				}
			}

			dst_data[dst_index * 3 + 0] = R / (reduce * reduce);
			dst_data[dst_index * 3 + 1] = G / (reduce * reduce);
			dst_data[dst_index * 3 + 2] = B / (reduce * reduce);
		}
	}
	reductionDst.update();
	//reductionDst.save("small.jpg");
}

//--------------------------------------------------------------
void ofApp::thresholding() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = otsuDst.getPixels().getData();

	double Pall = width * height;
	double Sbmax = 0;
	double Sb = 0;
	int t = 0;
	int lumin[256] = {};

	//輝度0～255のピクセル数をカウント
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = y * width + x;
			int R = src_data[index * 3 + 0];
			int G = src_data[index * 3 + 1];
			int B = src_data[index * 3 + 2];

			int gray = R * 0.299 + G * 0.587 + B * 0.114;

			lumin[gray]++;

		}
	}

	//大津の二値化アルゴリズム
	//Pall: 全画素数
	//P0, P1: クラスの画素数
	//R0 = P0 / Pall
	//R1 = P1 / Pall

	// M0, M1: クラス内の平均	
	//Sb^2 = R0 * R1 * (M0 - M1) ^2
	//Sb^2が最大なものが閾値t

	//最適な閾値t = 0となることはないため1からはじめる
	for (int i = 1; i < 256; i++) {
		double P0 = 0;
		double P1 = 0;
		double R0 = 0;
		double R1 = 0;
		double M0 = 0;
		double M1 = 0;

		//クラス１（1～tまで）
		for (int j = 1; j <= i; j++) {
			P0 += lumin[j];
			M0 += j * lumin[j];
		}
		R0 = P0 / Pall;
		M0 /= P0;

		//クラス２（t～255まで）
		for (int k = 255; k > i; k--) {
			P1 += lumin[k];
			M1 += k * lumin[k];
		}
		R1 = P1 / Pall;
		M1 /= P1;

		Sb = R0 * R1 * (M0 - M1) * (M0 - M1);

		if (Sb > Sbmax) {
			Sbmax = Sb;
			t = i;
		}
	}

	//二値化の適用
	//閾値t未満は黒
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = y * width + x;
			int R = src_data[index * 3 + 0];
			int G = src_data[index * 3 + 1];
			int B = src_data[index * 3 + 2];

			int gray = R * 0.299 + G * 0.587 + B * 0.114;

			if (gray >= t) {
				dst_data[index] = 255;
			}
			else if (gray < t) {
				dst_data[index] = 0;
			}

		}
	}
	otsuDst.update();
	//閾値確認用
	//ofLog() << "Otsu Threshold: " << t;
	//otsuDst.save("otsu.jpg");
}

//--------------------------------------------------------------
void ofApp::gray() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = grayDst.getPixels().getData();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = y * width + x;
			int R = src_data[index * 3 + 0];
			int G = src_data[index * 3 + 1];
			int B = src_data[index * 3 + 2];

			dst_data[index * 3 + 0] = R * 0.299 + G * 0.587 + B * 0.114;
			dst_data[index * 3 + 1] = R * 0.299 + G * 0.587 + B * 0.114;
			dst_data[index * 3 + 2] = R * 0.299 + G * 0.587 + B * 0.114;
		}
	}
	grayDst.update();
	//grayDst.save("gray.jpg");
}

//--------------------------------------------------------------
void ofApp::sepia() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = sepiaDst.getPixels().getData();

	for (int y = 0; y < height; y++) {
		for (int x = 0;x < width; x++) {
			int index = y * width + x;

			int R = src_data[index * 3 + 0];
			int G = src_data[index * 3 + 1];
			int B = src_data[index * 3 + 2];

			//https://qiita.com/yoya/items/9c237caf86ea5ade2617 のセピア調フィルタ
			int sepia_R = 0.393 * R + 0.769 * G + 0.189 * B;
			int sepia_G = 0.349 * R + 0.686 * G + 0.168 * B;
			int sepia_B = 0.272 * R + 0.534 * G + 0.131 * B;

			if (sepia_R > 255) {
				sepia_R = 255;
			}
			if (sepia_G > 255) {
				sepia_G = 255;
			}
			if (sepia_B > 255) {
				sepia_B = 255;
			}

			dst_data[index * 3 + 0] = sepia_R;
			dst_data[index * 3 + 1] = sepia_G;
			dst_data[index * 3 + 2] = sepia_B;
		}
	}
	sepiaDst.update();
}

//--------------------------------------------------------------
void ofApp::medianFilter() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = medianDst.getPixels().getData();
	//ピクセル取得 -> ソート -> 中央値を取得
	//今回は3×3のみに対応

	//（intと色データのペア）の可変長配列
	std::vector<std::pair<int, ofColor>> colorDict;
	//reverse()でサイズ指定（要素なしのメモリ確保）
	colorDict.reserve(9);

	for (int y = 1; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			colorDict.clear();
			int index = y * width + x;

			//3×3のソート
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					int dIndex = (y + dy) * width + (x + dx);

					int R = src_data[dIndex * 3 + 0];
					int G = src_data[dIndex * 3 + 1];
					int B = src_data[dIndex * 3 + 2];

					//static_cast<型>()で型を特定して変換できる
					int lumin = static_cast<int>(R * 0.299 + G * 0.587 + B * 0.114);

					//pairは{}
					colorDict.push_back({ lumin, ofColor(R, G, B)});
				}
			}
			//sort()はデフォルトで昇順にソート
			//pairの時はfirst優先でソート
			//secondのofColorは比較の定義がされていないため、以下の記述式により
			//firstのみで比較を行わせる（made by chatGPT）
			//これがないとsecondも比較しようとするためビルドエラーが発生
			std::sort(colorDict.begin(), colorDict.end(),
				[](const std::pair<int, ofColor>& a, const std::pair<int, ofColor>& b) {
					return a.first < b.first;
				});


			//.first, .secondでkey, valueを取得できる
			ofColor medianColor = colorDict[colorDict.size() / 2].second;

			dst_data[index * 3 + 0] = medianColor.r;
			dst_data[index * 3 + 1] = medianColor.g;
			dst_data[index * 3 + 2] = medianColor.b;
		}
	}

	medianDst.update();
	//medianDst.save("median.jpg");
}

//--------------------------------------------------------------
void ofApp::pixelSort() {
	unsigned char* src_data = src.getPixels().getData();
	unsigned char* dst_data = pixelDst.getPixels().getData();

	std::vector<std::pair<int, ofColor>> pixelDict;

	for (int y = 0; y < height; y++) {
		pixelDict.clear();

		for (int x = 0; x < width; x++) {
			int index = y * width + x;

			int R = src_data[index * 3 + 0];
			int G = src_data[index * 3 + 1];
			int B = src_data[index * 3 + 2];

			int lumin = static_cast<int>(R * 0.299 + G * 0.587 + B * 0.114);
			pixelDict.push_back({ lumin, ofColor(R, G, B) });
		}

		std::sort(pixelDict.begin(), pixelDict.end(),
			[](const std::pair<int, ofColor>& a, const std::pair<int, ofColor>& b) {
				return a.first < b.first;
			});

		for (int x = 0; x < width; x++) {
			int index = y * width + x;
			dst_data[index * 3 + 0] = pixelDict[x].second.r;
			dst_data[index * 3 + 1] = pixelDict[x].second.g;
			dst_data[index * 3 + 2] = pixelDict[x].second.b;
		}
	}

	pixelDst.update();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	//src.draw(0, 0);
	//dst.draw(0, 0);
	//nearDst.draw(0, 0);
	//reductionDst.draw(0, 0);
	//sepiaDst.draw(0, 0);
	//grayDst.draw(0, 0);
	//otsuDst.draw(0, 0);
	medianDst.draw(0, 0);
	//pixelDst.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
