# mediapipe/examples/desktop/airclass_hand_detection/BUILD

licenses(["notice"])

package(default_visibility = ["//mediapipe/examples:__subpackages__"])

cc_binary(
    name = "airclass_hand_detection",
    srcs = ["airclass_hand_detection_main.cc"],
    data = [
        "//mediapipe/modules/hand_landmark:hand_landmark_full.tflite",
        "//mediapipe/modules/palm_detection:palm_detection_full.tflite",
        ":airclass_hand_detection_cpu.pbtxt", # Local file
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:landmark_cc_proto",
        # This target provides HandLandmarkTrackingCpu AND its necessary dependencies like the renderer
        "//mediapipe/graphs/hand_tracking:desktop_tflite_calculators",
        # We also need PassThroughCalculator if it's used in the graph
        "//mediapipe/calculators/core:pass_through_calculator",
        # Your custom calculator
        "//mediapipe/examples/desktop/airclass_hand_detection/calculators:thumbs_up_detection_calculator",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
        "@com_google_absl//absl/strings",
    ],
)

# This makes the .pbtxt file available to be used as a data dependency.
exports_files(["airclass_hand_detection_cpu.pbtxt"])