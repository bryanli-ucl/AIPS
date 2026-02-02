#pragma once

#include <Arduino.h>

class dev_base {
    public:
    virtual ~dev_base() {}
    virtual bool begin()                  = 0;
    virtual void update()                 = 0;
    virtual bool is_enable()              = 0;
    virtual void enable(bool active)      = 0;
    virtual inline const char* get_name() = 0;
};

class device_manager {
    public:
    static device_manager& get_instance() {
        static device_manager instance;
        return instance;
    }

    device_manager(const device_manager&)            = delete;
    device_manager& operator=(const device_manager&) = delete;

    bool reg_device(dev_base* device, const char* name);
    dev_base* get_device(const char* name);

    void initialize_all();
    void enable_all();
    void disable_all();

    uint8_t get_device_count() const { return m_device_count; }
    void list_devices();

    void print_device_status();

    private:
    device_manager() : m_device_count(0) {}

    static const uint8_t MAX_DEVICES = 10;

    struct DeviceEntry {
        dev_base* device;
        char name[32];
        bool registered;
    };

    DeviceEntry m_devices[MAX_DEVICES];
    uint8_t m_device_count;

    int8_t find_device(const char* name);
};