#include <Arduino.h>
#include <ArduTFLite.h> // TensorFlow Lite for Arduino
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "gas_leakage_model.h" // Include the generated TFLite model file

// Increase tensor arena size for better memory allocation
constexpr int kTensorArenaSize = 16384; 
uint8_t tensor_arena[kTensorArenaSize];

// TensorFlow Lite model variables
tflite::MicroInterpreter* interpreter;
const tflite::Model* model;
tflite::AllOpsResolver resolver;

// Define the gas sensor pin
const int gasSensorPin = A0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Initializing TensorFlow Lite model...");

    // Load TensorFlow Lite model
    model = tflite::GetModel(gas_leakage_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Model schema version mismatch!");
        while (1);
    }

    // Initialize the interpreter
    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate memory for model tensors
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("AllocateTensors() failed! Not enough memory.");
        while (1);
    }

    Serial.println("TensorFlow Lite model initialized successfully!");
}

void loop() {
    // Read gas sensor value
    int sensorValue = analogRead(gasSensorPin);
    float normalized_input = sensorValue / 1024.0; // Normalize sensor input

    // Set input tensor
    float* input = interpreter->input(0)->data.f;
    *input = normalized_input;

    // Perform inference
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("Invoke() failed!");
        return;
    }

    // Retrieve output tensor7
    float* output = interpreter->output(0)->data.f;

    // Display results
    Serial.print("Gas Sensor Reading: ");
    Serial.print(sensorValue);
    Serial.print(" | Model Prediction: ");
    Serial.println(*output);

    // Trigger alert if gas level is above threshold
    if (*output > 0.8) { // Threshold for gas leakage detection
        Serial.println("Warning! Gas Leakage Detected!");
        // Add additional actions like buzzer, LED, or wireless alert here
    }

    delay(1000); // Wait before next reading
}
