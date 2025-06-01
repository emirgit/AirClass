#include <cstdlib>
#include <iostream>
#include <string>
#include <vector> // Required for std::vector if not included by others

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/absl_log.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing the CalculatorGraphConfig proto. "
          "If not specified, will use the default config.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not specified, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not specified, show result in a window.");

const char kDefaultGraphConfigFile[] =
    "mediapipe/examples/desktop/airclass_hand_detection/airclass_hand_detection_cpu.pbtxt";
const char kInputStream[] = "input_video";
const char kOutputStream[] = "output_video";
const char kGestureStream[] = "detected_gesture";
const char kWindowName[] = "AirClass Hand Detection";

absl::Status RunMPPGraph() {
  std::string calculator_graph_config_contents;
  std::string calculator_graph_config_file = absl::GetFlag(FLAGS_calculator_graph_config_file);
  if (calculator_graph_config_file.empty()) {
    calculator_graph_config_file = kDefaultGraphConfigFile;
  }

  MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
      calculator_graph_config_file, &calculator_graph_config_contents));
  ABSL_LOG(INFO) << "Get calculator graph config contents: "
                 << calculator_graph_config_contents;
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  ABSL_LOG(INFO) << "Initialize the calculator graph.";
  mediapipe::CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  ABSL_LOG(INFO) << "Initialize the camera or load the video.";
  cv::VideoCapture capture;
  const std::string input_video_path = absl::GetFlag(FLAGS_input_video_path);
  if (!input_video_path.empty()) {
    capture.open(input_video_path);
  } else {
    capture.open(0); // Try to open the default webcam
  }
  RET_CHECK(capture.isOpened()) << "Failed to open video source.";

  cv::VideoWriter writer;
  const std::string output_video_path = absl::GetFlag(FLAGS_output_video_path);
  bool save_video = !output_video_path.empty();

  if (!output_video_path.empty()) {
    int codec = cv::VideoWriter::fourcc('a', 'v', 'c', '1'); // H.264 codec
    double fps = capture.get(cv::CAP_PROP_FPS);
    if (fps <= 0) fps = 30; // Default fps if not available
    cv::Size frame_size(
        static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT)));
    writer.open(output_video_path, codec, fps, frame_size);
    RET_CHECK(writer.isOpened()) << "Failed to open video writer for " << output_video_path;
  } else {
    cv::namedWindow(kWindowName, cv::WINDOW_AUTOSIZE);
#if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS, 30);
#endif
  }

  ABSL_LOG(INFO) << "Start running the calculator graph.";
  MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller video_poller,
                     graph.AddOutputStreamPoller(kOutputStream));
  MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller gesture_poller,
                     graph.AddOutputStreamPoller(kGestureStream));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  ABSL_LOG(INFO) << "Start processing frames.";
  bool grab_frames = true;
  while (grab_frames) {
    cv::Mat camera_frame_raw;
    capture >> camera_frame_raw;
    if (camera_frame_raw.empty()) {
      if (!input_video_path.empty()) {
          ABSL_LOG(INFO) << "End of video.";
      } else {
          ABSL_LOG(ERROR) << "Failed to capture frame from camera.";
      }
      break;
    }
    cv::Mat camera_frame;
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    if (input_video_path.empty()) { // Flip for webcam
        cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
    }


    auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
        mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
        kInputStream, mediapipe::Adopt(input_frame.release())
                          .At(mediapipe::Timestamp(frame_timestamp_us))));

    mediapipe::Packet video_packet;
    if (!video_poller.Next(&video_packet)) break;
    auto& output_frame = video_packet.Get<mediapipe::ImageFrame>();

    mediapipe::Packet gesture_packet;
    if (gesture_poller.Next(&gesture_packet)) {
      std::string gesture = gesture_packet.Get<std::string>();
      if (!gesture.empty()) {
        // Add gesture text to the frame for display
        cv::Mat output_frame_mat_for_text = mediapipe::formats::MatView(&output_frame);
        cv::putText(output_frame_mat_for_text, gesture, cv::Point(20, 50), 
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        // std::cout << "Detected gesture: " << gesture << std::endl; // Keep if console output is also desired
      }
    }

    cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR); // Convert back for OpenCV display

    if (save_video) {
      writer.write(output_frame_mat);
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255) {
        grab_frames = false;
      }
    }
  }

  ABSL_LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();
  MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
  return graph.WaitUntilDone();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  absl::Status run_status = RunMPPGraph();
  if (!run_status.ok()) {
    ABSL_LOG(ERROR) << "Failed to run the graph: " << run_status.message();
    // Detailed error logging for graph initialization issues
    if (run_status.message().find("ValidatedGraphConfig Initialization failed") != std::string::npos) {
        ABSL_LOG(ERROR) << "This often means a calculator specified in the .pbtxt graph config "
                        << "file was not found. Ensure all custom calculators are correctly "
                        << "defined in a BUILD file and linked into the binary.";
    }
    return EXIT_FAILURE;
  } else {
    ABSL_LOG(INFO) << "Success!";
  }
  return EXIT_SUCCESS;
}