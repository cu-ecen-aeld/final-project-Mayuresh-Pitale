#include <iostream>
#include <fstream>
#include <pthread.h>
#include <mqueue.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <chrono>

#define QUEUE_NAME "/vibra_queue"
#define DEVICE_PATH "/dev/vibra_sensor"
#define MAX_SIZE 1024
#define MSG_STOP "EXIT"

// Total logging time (5 minutes = 300 seconds)
// Note: Change this to 10 for a quick test before doing the full 5 minutes!
#define LOG_DURATION_SEC 300 

pthread_mutex_t print_mutex;

// ---------------------------------------------------------
// THREAD 1: Real Sensor Data Collector
// ---------------------------------------------------------
void* sensor_thread(void* arg) {
    mqd_t mq = mq_open(QUEUE_NAME, O_WRONLY);
    
    int sensor_fd = open(DEVICE_PATH, O_RDONLY);
    if (sensor_fd < 0) {
        std::cerr << "Sensor Thread: Failed to open " << DEVICE_PATH << "! Is the kernel module loaded?" << std::endl;
        mq_send(mq, MSG_STOP, strlen(MSG_STOP) + 1, 0);
        return NULL;
    }

    auto start_time = std::chrono::steady_clock::now();
    uint8_t raw_data[6];

    pthread_mutex_lock(&print_mutex);
    std::cout << "[Sensor Thread] Started reading real data for " << LOG_DURATION_SEC << " seconds..." << std::endl;
    pthread_mutex_unlock(&print_mutex);

    while (true) {
        // Check if 5 minutes have passed
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
        if (elapsed >= LOG_DURATION_SEC) {
            break;
        }

        // Read exactly 6 bytes from our custom kernel driver
        // Because of our wait_queue, this automatically sleeps until the hardware interrupt fires!
        ssize_t bytes_read = read(sensor_fd, raw_data, 6);
        if (bytes_read == 6) {
            // Convert Little-Endian hex bytes to 16-bit signed integers
            int16_t x = (raw_data[1] << 8) | raw_data[0];
            int16_t y = (raw_data[3] << 8) | raw_data[2];
            int16_t z = (raw_data[5] << 8) | raw_data[4];

            // Format as CSV row
            char buffer[MAX_SIZE];
            sprintf(buffer, "%d,%d,%d", x, y, z);

            // Send to Inference Thread
            mq_send(mq, buffer, strlen(buffer) + 1, 0);
        }
    }

    close(sensor_fd);
    mq_send(mq, MSG_STOP, strlen(MSG_STOP) + 1, 0);
    return NULL;
}

// ---------------------------------------------------------
// THREAD 2: Machine Learning CSV Logger
// ---------------------------------------------------------
void* inference_thread(void* arg) {
    mqd_t mq = mq_open(QUEUE_NAME, O_RDONLY);
    
    std::ofstream csv_file("normal_motor_baseline.csv");
    if (!csv_file.is_open()) {
        std::cerr << "Inference Thread: Failed to create CSV file!" << std::endl;
        return NULL;
    }

    // Write CSV Header
    csv_file << "Accel_X,Accel_Y,Accel_Z\n";

    char buffer[MAX_SIZE + 1];
    int samples_recorded = 0;

    while (true) {
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            
            if (strncmp(buffer, MSG_STOP, strlen(MSG_STOP)) == 0) {
                break;
            }

            // Write the data to the CSV file
            csv_file << buffer << "\n";
            samples_recorded++;

            // Print progress every 1000 samples so we know it's working
            if (samples_recorded % 1000 == 0) {
                pthread_mutex_lock(&print_mutex);
                std::cout << "[Inference Thread] Logged " << samples_recorded << " samples to CSV..." << std::endl;
                pthread_mutex_unlock(&print_mutex);
            }
        }
    }

    csv_file.close();
    
    pthread_mutex_lock(&print_mutex);
    std::cout << "[Inference Thread] Finished! Total samples saved: " << samples_recorded << std::endl;
    pthread_mutex_unlock(&print_mutex);
    
    mq_close(mq);
    return NULL;
}

// ---------------------------------------------------------
// MAIN
// ---------------------------------------------------------
int main() {
    std::cout << "--- Smart Edge Sensor Node ---" << std::endl;
    pthread_mutex_init(&print_mutex, NULL);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
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

    std::cout << "App complete. Check 'normal_motor_baseline.csv' for data." << std::endl;
    return 0;
}
