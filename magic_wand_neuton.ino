/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "src/neuton.h"

#include "main_functions.h"

#include "accelerometer_handler.h"
#include "constants.h"
#include "gesture_predictor.h"

//#define NO_CALC

// Globals, used for compatibility with Arduino-style sketches.
namespace {
input_t* model_input = nullptr;
int window;
int num_features;
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Obtain pointer to the model's input
#ifndef NO_CALC
  model_input = neuton_model_get_inputs_ptr();
#else
  model_input = NULL;
#endif
  window = neuton_model_window_size();
  num_features = neuton_model_inputs_count();
    
  bool setup_status = SetupAccelerometer();
  if (!setup_status) {
    Serial.println("Set up failed");
  }
}

void loop() {

  // Attempt to read new data from the accelerometer.
  bool got_data =
      ReadAccelerometer(model_input, num_features, window);
  
  // If there was no new data, wait until next time.
  if (!got_data) return;

  // Run inference, and report any error.
  float *output = NULL;
  neuton_model_set_ready_flag();
  uint64_t start_time = micros();
#ifndef NO_CALC
  int8_t invoke_status = neuton_model_run_inference(NULL, &output);
#else
  int8_t invoke_status = 0;
#endif  
  uint64_t stop_time = micros();
  if (invoke_status != 0) {
    Serial.print("Invoke failed on index: ");
    Serial.println(begin_index);
    return;
  }

#if 0
  Serial.print("Model size: ");
  Serial.print(neuton_model_size_with_meta());
  Serial.print(" bytes\n");
#endif

#if 0
  Serial.print("Inference time: ");
  Serial.print(stop_time - start_time);
  Serial.print(" us\n");
#endif

#if 0
  Serial.print(output[0], 5);
  Serial.print(", ");
  Serial.print(output[1], 5);
  Serial.print(", ");
  Serial.print(output[2], 5);
  Serial.print(", ");
  Serial.print(output[3], 5);
  Serial.print("\n");
#endif

  // Analyze the results to obtain a prediction
  int gesture_index = PredictGesture(output);

    // Print some ASCII art for each gesture and control the LED.
  if (gesture_index == 0) {
    Serial.print(
        "WING:\n\r*         *         *\n\r *       * *       "
        "*\n\r  *     *   *     *\n\r   *   *     *   *\n\r    * *       "
        "* *\n\r     *         *\n\r");
  } else if (gesture_index == 1) {
    Serial.print(
        "RING:\n\r          *\n\r       *     *\n\r     *         *\n\r "
        "   *           *\n\r     *         *\n\r       *     *\n\r      "
        "    *\n\r");
  } else if (gesture_index == 2) {
    Serial.print(
        "SLOPE:\n\r        *\n\r       *\n\r      *\n\r     *\n\r    "
        "*\n\r   *\n\r  *\n\r * * * * * * * *\n\r");
  }
}
