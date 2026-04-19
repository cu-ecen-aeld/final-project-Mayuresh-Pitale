#include <iostream>
#include <pthread.h>
#include <mqueue.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <cmath>
#include <memory>

// TensorFlow Lite Headers
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#define QUEUE_NAME "/vibra_queue"
#define DEVICE_PATH "/dev/vibra_sensor"
#define MODEL_PATH "anomaly_model.tflite"

pthread_mutex_t print_mutex;

// Hardcoded values from your Python Colab training script
const float min_vals[3] = {-147.00, 3748.00, -15859.00};
const float scale_vals[3] = {0.001553, 0.002217, 0.001387};
const float THRESHOLD = 0.159229;

// Upgraded Message Queue Payload
struct SensorData {
    int16_t x, y, z;
    bool stop;
};

// ---------------------------------------------------------
// THREAD 1: Real Sensor Data Collector
// ---------------------------------------------------------
void* sensor_thread(void* arg) {
    mqd_t mq = mq_open(QUEUE_NAME, O_WRONLY);
    int sensor_fd = open(DEVICE_PATH, O_RDONLY);
    if (sensor_fd < 0) {
        std::cerr << "Sensor Thread: Failed to open driver!" << std::endl;
        SensorData stop_msg = {0, 0, 0, true};
        mq_send(mq, (char*)&stop_msg, sizeof(SensorData), 0);
        return NULL;
    }

    uint8_t raw_data[6];
    SensorData data;
    data.stop = false;

    pthread_mutex_lock(&print_mutex);
    std::cout << "[Sensor] Streaming live data to Inference Engine..." << std::endl;
    pthread_mutex_unlock(&print_mutex);

    // Run indefinitely until user presses Ctrl+C
    while (true) {
        if (read(sensor_fd, raw_data, 6) == 6) {
            data.x = (raw_data[1] << 8) | raw_data[0];
            data.y = (raw_data[3] << 8) | raw_data[2];
            data.z = (raw_data[5] << 8) | raw_data[4];
            
            mq_send(mq, (char*)&data, sizeof(SensorData), 0);
        }
    }

    close(sensor_fd);
    return NULL;
}

// ---------------------------------------------------------
// THREAD 2: TensorFlow Lite Edge Inference
// ---------------------------------------------------------
void* inference_thread(void* arg) {
    mqd_t mq = mq_open(QUEUE_NAME, O_RDONLY);
    
    // Load the TFLite Model
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(MODEL_PATH);
    if (!model) {
        std::cerr << "Inference: Failed to load " << MODEL_PATH << std::endl;
        return NULL;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    interpreter->AllocateTensors();

    float* input_tensor = interpreter->typed_input_tensor<float>(0);
    float* output_tensor = interpreter->typed_output_tensor<float>(0);

    SensorData data;
    int print_counter = 0;

    while (true) {
        ssize_t bytes_read = mq_receive(mq, (char*)&data, sizeof(SensorData), NULL);
        if (bytes_read == sizeof(SensorData)) {
            if (data.stop) break;

            // 1. Normalize the Input Data
            float in_x = (data.x - min_vals[0]) * scale_vals[0];
            float in_y = (data.y - min_vals[1]) * scale_vals[1];
            float in_z = (data.z - min_vals[2]) * scale_vals[2];

            // 2. Feed the Neural Network
            input_tensor[0] = in_x;
            input_tensor[1] = in_y;
            input_tensor[2] = in_z;

            // 3. Run Inference
            interpreter->Invoke();

            // 4. Calculate Mean Squared Error (MSE)
            float mse = (pow(in_x - output_tensor[0], 2) + 
                         pow(in_y - output_tensor[1], 2) + 
                         pow(in_z - output_tensor[2], 2)) / 3.0;

            // 5. Detect Anomalies (Print 4 times a second so terminal doesn't crash)
            print_counter++;
            if (print_counter >= 26) { // 104Hz / 4 = ~26 samples
                pthread_mutex_lock(&print_mutex);
                if (mse > THRESHOLD) {
                    std::cout << "\033[1;31m[ANOMALY DETECTED] MSE: " << mse << " > " << THRESHOLD << "\033[0m" << std::endl;
                } else {
                    std::cout << "\033[1;32m[NORMAL] MSE: " << mse << "\033[0m" << std::endl;
                }
                pthread_mutex_unlock(&print_mutex);
                print_counter = 0;
            }
        }
    }

    mq_close(mq);
    return NULL;
}

// ---------------------------------------------------------
// MAIN
// ---------------------------------------------------------
int main() {
    std::cout << "--- Smart Edge Anomaly Detector Initializing ---" << std::endl;
    pthread_mutex_init(&print_mutex, NULL);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(SensorData);
    attr.mq_curmsgs = 0;

    mq_unlink(QUEUE_NAME); 
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, sensor_thread, NULL);
    pthread_create(&t2, NULL, inference_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
    pthread_mutex_destroy(&print_mutex);
    return 0;
}
