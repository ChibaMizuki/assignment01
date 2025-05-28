#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//�Q�L���N�A�C�R��
	//src.load("gkyk.jpg");

	// �J�[�h�C���X�g
	//src.load("mzk.jpg");

	//�N���オ�菬�@�t
	src.load("kbs.jpg");

	//POPSONG
	//src.load("yk.jpg");

	width = src.getWidth();
	height = src.getHeight();

	//�F����̑傫��
	dx = 10;
	dst.allocate(width, height, OF_IMAGE_COLOR);

	//�g�嗦�i�����l�j
	expansion = 3;
	nearDst.allocate(width * expansion, height * expansion, OF_IMAGE_COLOR);

	//�k�����i1 / reduce, �����l�j
	reduce = 2.5;
	reductionDst.allocate(width / reduce, height / reduce, OF_IMAGE_COLOR);

	//�Z�s�A
	sepiaDst.allocate(width, height, OF_IMAGE_COLOR);

	//��Â̓�l��
	otsuDst.allocate(width, height, OF_IMAGE_GRAYSCALE);

	//�O���[�X�P�[���ϊ�
	grayDst.allocate(width, height, OF_IMAGE_COLOR);

	//���f�B�A���t�B���^�[
	medianDst.allocate(width, height, OF_IMAGE_COLOR);

	//�s�N�Z���\�[�g
	pixelDst.allocate(width, height, OF_IMAGE_COLOR);

	//�F����
	chromaticAberration();
	//�g��
	nearestNeighbor();
	//�k��
	reduction();
	//�Z�s�A���t�B���^�[
	sepia();
	//��Â̓�l��
	thresholding();
	//�O���[�X�P�[���ϊ�
	gray();
	//median�t�B���^�[
	medianFilter();
	//�s�N�Z���\�[�g
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

			//�Ԃ����ɂ��炷
			if (x + dx <= width) {
				R = src_data[(index * 3 + 0) + 3 * dx];
			}
			else {
				R = 0;
			}

			//���E�ɂ��炷
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

	//�P�x0�`255�̃s�N�Z�������J�E���g
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

	//��Â̓�l���A���S���Y��
	//Pall: �S��f��
	//P0, P1: �N���X�̉�f��
	//R0 = P0 / Pall
	//R1 = P1 / Pall

	// M0, M1: �N���X���̕���	
	//Sb^2 = R0 * R1 * (M0 - M1) ^2
	//Sb^2���ő�Ȃ��̂�臒lt

	//�œK��臒lt = 0�ƂȂ邱�Ƃ͂Ȃ�����1����͂��߂�
	for (int i = 1; i < 256; i++) {
		double P0 = 0;
		double P1 = 0;
		double R0 = 0;
		double R1 = 0;
		double M0 = 0;
		double M1 = 0;

		//�N���X�P�i1�`t�܂Łj
		for (int j = 1; j <= i; j++) {
			P0 += lumin[j];
			M0 += j * lumin[j];
		}
		R0 = P0 / Pall;
		M0 /= P0;

		//�N���X�Q�it�`255�܂Łj
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

	//��l���̓K�p
	//臒lt�����͍�
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
	//臒l�m�F�p
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

			//https://qiita.com/yoya/items/9c237caf86ea5ade2617 �̃Z�s�A���t�B���^
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
	//�s�N�Z���擾 -> �\�[�g -> �����l���擾
	//�����3�~3�݂̂ɑΉ�

	//�iint�ƐF�f�[�^�̃y�A�j�̉ϒ��z��
	std::vector<std::pair<int, ofColor>> colorDict;
	//reverse()�ŃT�C�Y�w��i�v�f�Ȃ��̃������m�ہj
	colorDict.reserve(9);

	for (int y = 1; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			colorDict.clear();
			int index = y * width + x;

			//3�~3�̃\�[�g
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					int dIndex = (y + dy) * width + (x + dx);

					int R = src_data[dIndex * 3 + 0];
					int G = src_data[dIndex * 3 + 1];
					int B = src_data[dIndex * 3 + 2];

					//static_cast<�^>()�Ō^����肵�ĕϊ��ł���
					int lumin = static_cast<int>(R * 0.299 + G * 0.587 + B * 0.114);

					//pair��{}
					colorDict.push_back({ lumin, ofColor(R, G, B)});
				}
			}
			//sort()�̓f�t�H���g�ŏ����Ƀ\�[�g
			//pair�̎���first�D��Ń\�[�g
			//second��ofColor�͔�r�̒�`������Ă��Ȃ����߁A�ȉ��̋L�q���ɂ��
			//first�݂̂Ŕ�r���s�킹��imade by chatGPT�j
			//���ꂪ�Ȃ���second����r���悤�Ƃ��邽�߃r���h�G���[������
			std::sort(colorDict.begin(), colorDict.end(),
				[](const std::pair<int, ofColor>& a, const std::pair<int, ofColor>& b) {
					return a.first < b.first;
				});


			//.first, .second��key, value���擾�ł���
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
