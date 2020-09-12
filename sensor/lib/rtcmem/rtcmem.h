extern "C" {
    #include "user_interface.h" // this is for the RTC memory read/write functions
}
#define RTC_MEMORY_START 65
#define RTC_BLOCK_SIZE 4        // 4 bytes = 32bit

// assert(sizeof (DWORD) == sizeof (BYTE[RTC_BLOCK_SIZE]));   // Sanity check
union rtcBufferType {           // 32 bit rtc mem buffer as dword and 4 bytes
    uint32_t dw;
    struct {
         char b[RTC_BLOCK_SIZE];
    } bytes;
};

class rtcmem {
  public:
    uint32_t read32 (int);
    void write32 (int, uint32_t);
};

uint32_t rtcmem::read32 (int index) {
    uint32_t buffer;
    system_rtc_mem_read(index + RTC_MEMORY_START, &buffer, sizeof(buffer));
    return buffer;
}

void rtcmem::write32 (int index, uint32_t data) {
    system_rtc_mem_write(index + RTC_MEMORY_START, &data, sizeof(data));
}