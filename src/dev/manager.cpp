#include "dev/manager.hpp"

#include "logger.hpp"

bool device_manager::reg_device(dev_base* device, const char* name) {
    if (m_device_count >= MAX_DEVICES) {
        LOG_ERROR("Maximum devices reached! (max: {})", MAX_DEVICES);
        return false;
    }

    if (device == nullptr || name == nullptr) {
        LOG_ERROR("Invalid device or name pointer");
        return false;
    }

    if (find_device(name) >= 0) {
        LOG_WARN("Device '{}' already registered", name);
        return false;
    }

    m_devices[m_device_count].device = device;
    strncpy(m_devices[m_device_count].name, name, 31);
    m_devices[m_device_count].name[31]   = '\0';
    m_devices[m_device_count].registered = true;
    m_device_count++;

    LOG_INFO("Registered device: {}", name);

    return true;
}

dev_base* device_manager::get_device(const char* name) {
    int8_t index = find_device(name);
    if (index >= 0) {
        LOG_DEBUG("Found device: {}", name);
        return m_devices[index].device;
    }
    LOG_WARN("Device '{}' not found", name);
    return nullptr;
}

void device_manager::initialize_all() {
    LOG_INFO("Initializing all devices...");

    uint8_t success_count = 0;
    uint8_t failed_count  = 0;

    for (uint8_t i = 0; i < m_device_count; i++) {
        if (m_devices[i].registered && m_devices[i].device != nullptr) {
            LOG_DEBUG("Initializing device: {}", m_devices[i].name);

            if (m_devices[i].device->begin()) {
                LOG_INFO("Device '{}' initialized successfully", m_devices[i].name);
                success_count++;
            } else {
                LOG_ERROR("Device '{}' initialization failed", m_devices[i].name);
                failed_count++;
            }
        }
    }

    LOG_INFO("Initialization complete: {} success, {} failed", success_count, failed_count);
}

void device_manager::enable_all() {
    LOG_INFO("Enabling all devices...");

    for (uint8_t i = 0; i < m_device_count; i++) {
        if (m_devices[i].registered && m_devices[i].device != nullptr) {
            m_devices[i].device->enable(true);
            LOG_DEBUG("Enabled device: {}", m_devices[i].name);
        }
    }
}

void device_manager::disable_all() {
    LOG_INFO("Disabling all devices...");

    for (uint8_t i = 0; i < m_device_count; i++) {
        if (m_devices[i].registered && m_devices[i].device != nullptr) {
            m_devices[i].device->enable(false);
            LOG_DEBUG("Disabled device: {}", m_devices[i].name);
        }
    }
}

void device_manager::list_devices() {
    LOG_INFO("========== Device List ==========");
    LOG_INFO("Total devices: {}", m_device_count);
    LOG_INFO("---------------------------------");

    for (uint8_t i = 0; i < m_device_count; i++) {
        if (m_devices[i].registered) {
            const char* status = m_devices[i].device->is_enable() ? "ACTIVE" : "INACTIVE";
            LOG_INFO("{}. {} - {}", i + 1, m_devices[i].name, status);
        }
    }

    LOG_INFO("=================================");
}

void device_manager::print_device_status() {
    LOG_INFO("========== Device Status ==========");

    for (uint8_t i = 0; i < m_device_count; i++) {
        if (m_devices[i].registered && m_devices[i].device != nullptr) {
            const char* status = m_devices[i].device->is_enable() ? "ACTIVE" : "INACTIVE";
            LOG_INFO("{}: {}", m_devices[i].name, status);
        }
    }

    LOG_INFO("===================================");
}

int8_t device_manager::find_device(const char* name) {
    return 0;
}
