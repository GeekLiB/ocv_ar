#include "detect.h"

using namespace ocv_ar;

#pragma mark public methods

Detect::Detect() {
    prepared = false;
    inputFrameCvtType = -1;
    outFrameProcLvl = DEFAULT;
    
    inFrameOrigGray = NULL;
    inFrame = NULL;
    outFrame = NULL;
    
    downsampleSizeW = downsampleSizeH = 0;
    
#if !defined(OCV_AR_CONF_DOWNSAMPLE) && defined(OCV_AR_CONF_RESIZE_W) && defined(OCV_AR_CONF_RESIZE_H)
    downsampleSizeW = OCV_AR_CONF_RESIZE_W;
    downsampleSizeH = OCV_AR_CONF_RESIZE_H;
#endif
}

Detect::~Detect() {
    if (inFrameOrigGray) delete inFrameOrigGray;
    if (inFrame) delete inFrame;
    if (outFrame) delete outFrame;
}

void Detect::prepare(int frameW, int frameH, int frameChan, int cvtType) {
    assert(frameW > 0 && frameH > 0 && (frameChan == 1 || frameChan == 3 || frameChan == 4));
    
    // alloc mem for orig. sized image in grayscale format
    if (inFrameOrigGray) delete inFrameOrigGray;
    inFrameOrigGray = new cv::Mat(frameH, frameW, CV_8UC1);
    
    // alloc mem for downsampled image in grayscale format
#ifdef OCV_AR_CONF_DOWNSAMPLE
    int frac = pow(2, OCV_AR_CONF_DOWNSAMPLE);
    int downW = frameW / frac;
    int downH = frameH / frac;

    if (inFrame) delete inFrame;
    
    inFrame = new cv::Mat(downH, downW, CV_8UC1);
#elif defined(OCV_AR_CONF_RESIZE_W) && defined(OCV_AR_CONF_RESIZE_H)
    assert(downsampleSizeW <= frameW && downsampleSizeH <= frameH);
    if (!inFrame) {
        inFrame = new cv::Mat(downsampleSizeH, downsampleSizeW, CV_8UC1);
    }
#else
#error Either OCV_AR_CONF_DOWNSAMPLE or OCV_AR_CONF_RESIZE_W/H must be defined.
#endif
    
    if (cvtType < 0) {  // guess color convert type
        if (frameChan == 3) {
            cvtType = CV_RGB2GRAY;
        } else if (frameChan == 4) {
            cvtType = CV_RGBA2GRAY;
        }
    }
    
    if (frameChan == 1) {
        cvtType = -1;   // means we don't need to call cv::cvtColor
    }
    
    // set properties
    inputFrameCvtType = cvtType;
    prepared = true;
    
    cout << "ocv_ar::Detect - prepared for frames: "
         << frameW << "x" << frameH << ", " << frameChan << " channels" << endl;
    cout << "ocv_ar::Detect - input frame color convert type: " << inputFrameCvtType << endl;
}

void Detect::setCamIntrinsics(cv::Mat &camIntrinsics) {
    
}

void Detect::setFrameOutputLevel(FrameProcLevel level) {
    assert(prepared);
    
    if (outFrameProcLvl == level) return; // no change, no op.
    
    outFrameProcLvl = level;
    
    int outW, outH;
    
    outW = inFrame->cols;
    outH = inFrame->rows;
    
    if (outFrame) {
        if (outW == outFrame->cols && outH == outFrame->rows) return;   // no change in output frame size
        
        delete outFrame;
    }
    
    outFrame = new cv::Mat(outH, outW, CV_8UC1);
    
    cout << "ocv_ar::Detect - set output frame level: "
         << level << " (output frame size " << outW << "x" << outH << ")" << endl;
}

void Detect::setInputFrame(cv::Mat *frame) {
    assert(prepared && frame);
    
    if (inputFrameCvtType >= 0) {   // convert to grayscale
        cv::cvtColor(*frame, *inFrameOrigGray, inputFrameCvtType);
    }
}

void Detect::processFrame() {
    preprocess();
    performThreshold();
}

cv::Mat *Detect::getOutputFrame() const {
    if (outFrameProcLvl == DEFAULT) return NULL;

    return outFrame;
}

#pragma mark private methods

void Detect::preprocess() {
#ifdef OCV_AR_CONF_DOWNSAMPLE
    for (int i = 0; i < OCV_AR_CONF_DOWNSAMPLE; i++) {
        cv::pyrDown(*inFrameOrigGray, *inFrame);
        
        inFrameOrigGray = inFrame;
    }
#elif defined(OCV_AR_CONF_RESIZE_W) && defined(OCV_AR_CONF_RESIZE_H)
    cv::resize(*inFrameOrigGray, *inFrame, cv::Size(OCV_AR_CONF_RESIZE_W, OCV_AR_CONF_RESIZE_H));
#else
#error Either OCV_AR_CONF_DOWNSAMPLE or OCV_AR_CONF_RESIZE_W/H must be defined.
#endif
    
    setOutputFrameOnCurProcLevel(PREPROC, inFrame);
}

void Detect::performThreshold() {
	cv::adaptiveThreshold(*inFrame,
                          *inFrame,
                          255,
                          cv::ADAPTIVE_THRESH_MEAN_C, // ADAPTIVE_THRESH_GAUSSIAN_C
                          cv::THRESH_BINARY_INV,
                          OCV_AR_CONF_THRESH_BLOCK_SIZE,
                          OCV_AR_CONF_THRESH_C);
    
    setOutputFrameOnCurProcLevel(THRESH, inFrame);
}

//void Detect::threshPostProc() {
//    
//}

void Detect::findContours() {
    
}

void Detect::findMarkerCandidates() {
    
}

void Detect::checkMarkerCandidates() {
    
}

void Detect::discardDuplicateMarkers() {
    
}

void Detect::estimatePositions() {
    
}

void Detect::setOutputFrameOnCurProcLevel(FrameProcLevel curLvl, cv::Mat *srcFrame) {
    assert(srcFrame);
    
    if (curLvl == outFrameProcLvl) {
        srcFrame->copyTo(*outFrame);
    }
}

int Detect::readMarkerCode(cv::Mat &img, int *validRot) {
    return 0;
}

bool Detect::checkMarkerCode(const cv::Mat &m, int dir) const {
    return false;
}

int Detect::markerCodeToId(const cv::Mat &m, int dir) const {
    return 0;
}

void Detect::drawMarker(cv::Mat &img, const Marker &m) {
    
}