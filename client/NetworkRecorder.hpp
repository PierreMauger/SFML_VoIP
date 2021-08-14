#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <atomic>
#include <condition_variable>
#include <cassert>
#include <mutex>
#include <queue>
#include <iostream>

struct Samples {
    Samples(sf::Int16 const* ss, std::size_t count) {
        samples.reserve(count);
        std::copy_n(ss, count, std::back_inserter(samples));
    }
    Samples() {}

    std::vector<sf::Int16> samples;
};

class NetworkRecorder : private sf::SoundRecorder, private sf::SoundStream {
public:

    sf::TcpSocket socket;

    void start() {
        sf::SoundRecorder::start();
        sf::SoundStream::initialize(sf::SoundRecorder::getChannelCount(), sf::SoundRecorder::getSampleRate());

        sf::SoundStream::play();
    }

    void stop() {
        sf::SoundRecorder::stop();
        sf::SoundStream::stop();
    }

    void receiveData(sf::Packet receivePacket) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            const sf::Int16* samples = reinterpret_cast<const sf::Int16*>(static_cast<const char*>(receivePacket.getData()) + sizeof(int));
            std::size_t sampleCount = (receivePacket.getDataSize() - sizeof(int)) / sizeof(sf::Int16);
            data.emplace(samples, sampleCount);
        }
    }

    void mute() {
        isMuted = !isMuted;
    }

    bool isRunning() {
        return isRecording;
    }

    ~NetworkRecorder() {
        stop();
    }

protected:

    bool onProcessSamples(sf::Int16 const* samples, std::size_t sampleCount) override {
        int status = 0;
        //      to hear yourself
        // {
            // std::lock_guard<std::mutex> lock(mutex);
            // data.emplace(samples, sampleCount);
        // }
        if (!isMuted) {
            packet.clear();
            packet << status;
            packet.append(samples, sampleCount * sizeof(sf::Int16));
            socket.send(packet);
        }
        cv.notify_one();
        return true;
    }

    bool onStart() override {
        isRecording = true;
        std::queue<Samples> empty;
        data.swap(empty);
        return true;
    }

    void onStop() override {
        isRecording = false;
        cv.notify_one();
        std::queue<Samples> empty;
        data.swap(empty);
    }

protected:

    bool onGetData(Chunk& chunk) override {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]{ return !isRecording || !data.empty(); });

        if (!isRecording)
            return false;
        else {
            assert(!data.empty());

            playingSamples.samples = std::move(data.front().samples);
            data.pop();
            chunk.sampleCount = playingSamples.samples.size();
            chunk.samples = playingSamples.samples.data();
            return true;
        }
    }

    void onSeek(sf::Time) override { /* Not supported, silently does nothing. */ }

private:
    std::atomic<bool> isRecording{false};
    bool isMuted = false;
    std::mutex mutex; // protects `data`
    std::condition_variable cv; // notify consumer thread of new samples
    std::queue<Samples> data; // samples come in from the recorder, and popped by the output stream
    Samples playingSamples; // used by the output stream.
    sf::Packet packet; // packet sended to server
};
