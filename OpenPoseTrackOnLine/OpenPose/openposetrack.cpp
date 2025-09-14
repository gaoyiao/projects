//
// Created by 高一奥 on 2025/6/8.
//
#include "openposetrack.h"
//#include "utils.h"


OpenPoseTrack::OpenPoseTrack(int h, int w, int tnum, float th, std::string device, std::string camurl) :
        inputNetHeight(h),
        inputNetWidth(w),
        thread_nums(tnum),
        threshold(th),
        deviceName(device),
        camera_url(camurl),
        pool(tnum),
        Nets(std::vector<cv::dnn::Net>(tnum)){
    //pool.init();
    netInit();

}

void OpenPoseTrack::netInit() {
    for(int i = 0; i < Nets.size(); ++i){
        Nets.at(i) = generateNet();
    }
}

OpenPoseTrack::~OpenPoseTrack()
{
    pool.shutdown();
}


// void OpenPoseTrack::handleRawImages(Server& server, int sock){
//     cv::Mat raw_image;
//     generateNet();
//     //int m_id = 0;
//     int count = 0;
//     while(true) {
//         //std::cout << show_images.size() << std::endl;
//         {
//             std::unique_lock <std::mutex> lock(lock_raw);
//             if (isRuning && raw_images.empty()) {
//                 m_condition_lock.wait(lock);
//                 //std::cout << "raw_images is empty, but still running" << std::endl;
//                 continue;
//             } else if (!isRuning && raw_images.empty()) {
//                 std::cout << "done" << std::endl;
//                 break;
//             }
//             raw_image = raw_images.front();
//             raw_images.pop_front();
//         }
// #ifdef SERIAL
//         cv::Mat out_end = handle(raw_image, this->Nets[0]);
//         cv::imwrite("../images/" + std::to_string(count) + ".jpg", out_end);
// #elif defined(PARALLEL_FUTURE)
//         //pool.submit(handle, raw_image);
//         //if(m_id >= thread_nums) m_id = 0;
//         keyPointfutures.push_back(
//                 pool.submit([this, raw_image]() -> cv::Mat
//                 {
//                     int m_id = pool.getThreadIndex(std::this_thread::get_id());
//                     return this->handle(raw_image, this->Nets[m_id]);})
//                 );
// #elif defined(PARALLEL_CALLBACK)
//         pool.submit_void(
//             [this, raw_image, count]{
//                 int m_id = pool.getThreadIndex(std::this_thread::get_id());
//                 cv::Mat out = this->handle(raw_image, this->Nets[m_id]);
//                 this->callBack(out, count);
//             }
//         );
// #endif
//         std::cout << "handle ... " << count++ << std::endl;
//         //++m_id;

//     }
//     std::cout << "handle over" << std::endl;
// }



void OpenPoseTrack::handleRawImages(Server& server, int sock){
    cv::Mat raw_image;
    //generateNet();
    //int m_id = 0;
    int count = 0;
    while(true) {
        std::cout << "curcular is working" << std::endl;
        {
            std::unique_lock <std::mutex> lock(*(server.getMatLock(sock))); //这里面的lock_raw应该由server提供，e.g server.getLock()
            if (isRuning && server.getMats(sock).empty()) {
                std::cout << "raw_images is empty, but still running" << std::endl;
                //m_condition_lock.wait(lock); 
                (server.getMatConditions(sock))->wait(lock);
                
                continue;
            } else if (!isRuning && server.getMats(sock).empty()) {
                std::cout << "done" << std::endl;
                break;
            }
            std::cout << "begin handle" << std::endl;
            raw_image = server.getMats(sock).front();
            server.getMats(sock).pop();
        }
        std::cout << "get input raw is Working" << std::endl;
#ifdef SERIAL
        std::cout << "It's working with serial mode" << std::endl;
        cv::Mat out_end = handle(raw_image, this->Nets[0]);
        cv::imwrite("../images/" + std::to_string(count) + ".jpg", out_end);
#elif defined(PARALLEL_FUTURE)
        //pool.submit(handle, raw_image);
        //if(m_id >= thread_nums) m_id = 0;
        keyPointfutures.push_back(
                pool.submit([this, raw_image]() -> cv::Mat
                {
                    int m_id = pool.getThreadIndex(std::this_thread::get_id());
                    return this->handle(raw_image, this->Nets[m_id]);})
                );
#elif defined(PARALLEL_CALLBACK)
        pool.submit_void(
            [this, raw_image, count]{
                int m_id = pool.getThreadIndex(std::this_thread::get_id());
                cv::Mat out = this->handle(raw_image, this->Nets[m_id]);
                this->callBack(out, count);
            }
        );
#endif
        std::cout << "handle ... " << count++ << std::endl;
        //++m_id;

    }
    std::cout << "handle over" << std::endl;
}



void OpenPoseTrack::shows(){
    cv::Mat showImage;
    while(isRuning){
        lock_shows.lock();
        if(show_images.empty()){
            lock_shows.unlock();
            continue;
        }
        showImage = show_images.front();
        show_images.pop_front();
        lock_shows.unlock();
        cv::imshow("showsWindow", showImage);
        if(cv::waitKey(1) == 'q'){
            isRuning = false;
            break;
        }
    }
    cv::destroyWindow("showsWindow");
    std::cout << "Whole System is Closed!" << std::endl;
}


cv::dnn::Net OpenPoseTrack::generateNet(){
    cv::dnn::Net net = cv::dnn::readNetFromCaffe(protoFile, weightsFile);

    if(deviceName == "CPU"){
        std::cout << "Using CPU Device" << std::endl;
        net.setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
    }
    else if(deviceName == "GPU"){
        std::cout << "Using GPU Device" << std::endl;
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }
    return net;
}

cv::Mat OpenPoseTrack::transInputSize(cv::Mat frame){
    cv::Mat inpBlob = cv::dnn::blobFromImage(frame, 1.0/255, cv::Size(inputNetWidth, inputNetHeight),
                                             cv::Scalar(0, 0, 0), false, false);

    return inpBlob;
}

 cv::Mat OpenPoseTrack::handle(cv::Mat image, cv::dnn::Net net){
    cv::Mat image_clone = image.clone();
    cv::Mat input = transInputSize(image_clone);
    if(input.empty()) std::cout << "input empty" << std::endl;
    net.setInput(input);
    cv::Mat output = net.forward();
    std::vector<cv::Point2f> keyPoints = keyPointsDetection(output, image_clone);
    cv::Mat out_image = generateKeypointMap(image_clone, keyPoints).clone();
    //keypoint_images.push_back(image);
    return out_image;

}

std::vector<cv::Point2f> OpenPoseTrack::keyPointsDetection(cv::Mat netOutput, cv::Mat oriImage){

    std::vector<cv::Point2f> keyPoints;

    int H = netOutput.size[2];
    int W = netOutput.size[3];

    for(int i = 0; i < nPoints; ++i){
        //cv::Mat probMap(H, W, CV_32F, netOutput.ptr(0, i));
        cv::Mat probMap = cv::Mat(H, W, CV_32F, netOutput.ptr(0, i)).clone();
        cv::Point2f p(-1, -1);
        cv::Point maxLoc;
        double prob;
        cv::minMaxLoc(probMap, 0, &prob, 0, &maxLoc);
        if(prob > threshold){
            p = maxLoc;
            p.x *= (float)(oriImage.cols) / W;
            p.y *= (float)(oriImage.rows) / H;
        }
        keyPoints.push_back(p);
    }
    return keyPoints;
}

cv::Mat OpenPoseTrack::generateKeypointMap(cv::Mat image, std::vector<cv::Point2f> keyPoints){
    for(int i = 0; i < nPoints - 1; ++i){
        cv::Point2f partA = keyPoints.at(POSE_PAIRS[i][0]);
        cv::Point2f partB = keyPoints.at(POSE_PAIRS[i][1]);
        if(partA.x <= 0 || partA.y <= 0 || partB.x <= 0 || partB.y <= 0) continue;
        cv::line(image, partA, partB, cv::Scalar(0, 255, 255), 8);
        cv::circle(image, partA, 8, cv::Scalar(0, 0, 255), -1);
        cv::circle(image, partB, 8, cv::Scalar(0, 0, 255), -1);
    }

    return image;
}

void OpenPoseTrack::callBack(cv::Mat& image, int count){
    cv::imwrite("../images/" + std::to_string(count) + ".jpg", image);
    {
        std::unique_lock<std::mutex> lock(m_lock_handled);
        m_handled_images.emplace(std::move(image));
        m_condition_handled.notify_one();
    }
}


void OpenPoseTrack::show() {
    cv::Mat show_image(1920, 1080, CV_8UC3, cv::Scalar(0, 0, 255));
    //cv::Mat show_image;
    while(true){
        {
            cv::imshow("handle", show_image);
            if(cv::waitKey(1) == 'q') return;
            std::unique_lock<std::mutex> lock(m_lock_handled);
            m_condition_handled.wait(lock, [this]() { return !m_handled_images.empty();});
            if(m_handled_images.empty()) continue;
            show_image = std::move(m_handled_images.front());
            m_handled_images.pop();
            
            //std::cout << "handle show ....................." << std::endl;
        }
    }
}




