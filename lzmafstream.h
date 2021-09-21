// SPDX-License-Identifier: BSD-2-Clause
#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif
#if (__cplusplus < 201402L)
#error "This header requires at least C++14"
#endif
#if (__cplusplus == 201703L)
#define _CXX_HAS_FILESYSTEM
#endif

#include <algorithm>
#include <istream>
#include <memory>
#include <ostream>
#include <streambuf>
#include <string>
#include <thread>
#include <utility>
#ifdef _CXX_HAS_FILESYSTEM
#include <filesystem>
#endif

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lzma.h>

namespace lzma {
class ifbuf : public std::streambuf {
  private:
    struct munmapper {
        void operator()(void* ptr) const noexcept;
        size_t len;
    }; // struct munmapper
  public:
    ifbuf& operator=(const ifbuf&) = delete;
    ifbuf& operator=(ifbuf&&) = default;

  public:
    ifbuf() noexcept;

    ifbuf(const ifbuf&) = delete;
    ifbuf(ifbuf&&) = default;

    ~ifbuf();

  public:
    bool is_open() const noexcept;
    bool open(const char* f, size_t bs) noexcept;
    void close() noexcept;
    uint64_t progress() noexcept;

  private:
    size_t avail_bytes() const noexcept;
    int_type underflow() override;
    int_type uflow() override;
    std::streamsize xsgetn(char_type* s, std::streamsize count) override;

  private:
    bool open_;
    int fd_;
    std::unique_ptr<void, munmapper> mem_;
    lzma_stream ls_;
    std::unique_ptr<uint8_t[]> b_;
    size_t bs_;
    size_t co_;
}; // class ifbuf
class ofbuf : public std::streambuf {
  public:
    ofbuf& operator=(const ofbuf&) = delete;
    ofbuf& operator=(ofbuf&&) = default;

  public:
    ofbuf() noexcept;

    ofbuf(const ofbuf&) = delete;
    ofbuf(ofbuf&&) = default;

    ~ofbuf();

  public:
    bool is_open() const noexcept;
    bool open(const char* f, uint32_t preset, bool mt, size_t bs) noexcept;
    void close() noexcept;
    uint64_t progress() noexcept;

  private:
    int sync() override;
    int_type overflow(int_type ch = traits_type::eof()) override;
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;

  private:
    bool open_;
    int fd_;
    lzma_stream ls_;
    std::unique_ptr<uint8_t[]> ib_;
    std::unique_ptr<uint8_t[]> ob_;
    size_t bs_;
}; // class ofbuf
class ifstream : public std::istream {
  public:
    ifstream() noexcept;
    ifstream(const char* f, size_t bs = 1048576) noexcept;
    ifstream(const std::string& s, size_t bs = 1048576) noexcept;
#ifdef _CXX_HAS_FILESYSTEM
    ifstream(const std::filesystem::path& p, size_t bs = 1048576) noexcept;
#endif

  public:
    bool is_open() const noexcept;
    void open(const char* f, size_t bs = 1048576) noexcept;
    void open(const std::string& s, size_t bs = 1048576) noexcept;
#ifdef _CXX_HAS_FILESYSTEM
    void open(const std::filesystem::path& p, size_t bs = 1048576) noexcept;
#endif
    void close() noexcept;
    uint64_t progress() noexcept;

  private:
    ifbuf ifb_;
}; // class ifstream
class ofstream : public std::ostream {
  public:
    ofstream() noexcept;
    ofstream(const char* f, uint32_t preset = LZMA_PRESET_DEFAULT,
             size_t bs = 1048576) noexcept;
    ofstream(const std::string& s, uint32_t preset = LZMA_PRESET_DEFAULT,
             size_t bs = 1048576) noexcept;
#ifdef _CXX_HAS_FILESYSTEM
    ofstream(const std::filesystem::path& p,
             uint32_t preset = LZMA_PRESET_DEFAULT,
             size_t bs = 1048576) noexcept;
#endif

  public:
    bool is_open() const noexcept;
    void open(const char* f, uint32_t preset = LZMA_PRESET_DEFAULT,
              size_t bs = 1048576) noexcept;
    void open(const std::string& s, uint32_t preset = LZMA_PRESET_DEFAULT,
              size_t bs = 1048576) noexcept;
#ifdef _CXX_HAS_FILESYSTEM
    void open(const std::filesystem::path& p,
              uint32_t preset = LZMA_PRESET_DEFAULT,
              size_t bs = 1048576) noexcept;
#endif
    void close() noexcept;
    uint64_t progress() noexcept;

  private:
    ofbuf ofb_;
}; // class ofstream
class mtofstream : public std::ostream {
  public:
    mtofstream() noexcept;
    mtofstream(const char* f, uint32_t preset = LZMA_PRESET_DEFAULT,
               size_t bs = 1048576) noexcept;
    mtofstream(const std::string& s, uint32_t preset = LZMA_PRESET_DEFAULT,
               size_t bs = 1048576) noexcept;
#ifdef _CXX_HAS_FILESYSTEM
    mtofstream(const std::filesystem::path& p,
               uint32_t preset = LZMA_PRESET_DEFAULT,
               size_t bs = 1048576) noexcept;
#endif

  public:
    bool is_open() const noexcept;
    void open(const char* f, uint32_t preset = LZMA_PRESET_DEFAULT,
              size_t bs = 1048576) noexcept;
    void open(const std::string& s, uint32_t preset = LZMA_PRESET_DEFAULT,
              size_t bs = 1048576) noexcept;
#ifdef _CXX_HAS_FILESYSTEM
    void open(const std::filesystem::path& p,
              uint32_t preset = LZMA_PRESET_DEFAULT,
              size_t bs = 1048576) noexcept;
#endif
    void close() noexcept;
    uint64_t progress() noexcept;

  private:
    ofbuf ofb_;
}; // class mtofstream

// begin ifbuf::munmapper implementation
void ifbuf::munmapper::operator()(void* ptr) const noexcept {
    if (!ptr)
        return;
    munmap(ptr, len);
}
// end ifbuf::munmapper implementation
// begin ifbuf implementation
ifbuf::ifbuf() noexcept
    : open_(false), fd_(-1), mem_(nullptr), ls_(LZMA_STREAM_INIT), b_(nullptr),
      bs_(0), co_(0) {}
ifbuf::~ifbuf() { close(); }

bool ifbuf::is_open() const noexcept { return open_; }
bool ifbuf::open(const char* f, size_t bs) noexcept {
    if (open_)
        close();
    // open file
    fd_ = ::open(f, O_RDONLY);
    if (fd_ == -1)
        return false;
    // get file size
    struct stat st;
    fstat(fd_, &st);
    size_t flen = static_cast<size_t>(st.st_size);
    // memmap file
    munmapper mu = {flen};
    void* mem = mmap(nullptr, flen, PROT_READ, MAP_PRIVATE, fd_, 0);
    if (!mem)
        return false;
    mem_ = std::unique_ptr<void, munmapper>(mem, std::move(mu));
    // set up read buffer
    b_ = std::make_unique<uint8_t[]>(bs);
    bs_ = bs;
    co_ = 0;
    // init decoder
    ls_ = LZMA_STREAM_INIT;
    auto ret = lzma_stream_decoder(&ls_, UINT64_MAX, LZMA_CONCATENATED);
    if (ret != LZMA_OK)
        return false;
    ls_.next_in = reinterpret_cast<uint8_t*>(mem_.get());
    ls_.avail_in = flen;
    ls_.next_out = b_.get();
    ls_.avail_out = bs_;
    // do initial decompress to fill read buffer
    ret = lzma_code(&ls_, LZMA_RUN);
    if (ret != LZMA_OK)
        return false;
    open_ = true;
    return true;
}
void ifbuf::close() noexcept {
    if (!open_)
        return;
    ::close(fd_);
    fd_ = -1;
    auto ret = lzma_code(&ls_, LZMA_FINISH);
    lzma_end(&ls_);
    open_ = false;
}
uint64_t ifbuf::progress() noexcept {
    if (!open_)
        return 0;
    uint64_t prog_in, prog_out;
    lzma_get_progress(&ls_, &prog_in, &prog_out);
    return prog_in;
}

size_t ifbuf::avail_bytes() const noexcept {
    auto avail_size = bs_ - ls_.avail_out;
    return (avail_size - co_);
}
ifbuf::int_type ifbuf::underflow() {
    if (avail_bytes()) {
        return traits_type::to_int_type(b_[co_]);
    } else {
        if (ls_.avail_in) {
            co_ = 0;
            ls_.next_out = b_.get();
            ls_.avail_out = bs_;
            auto ret = lzma_code(&ls_, LZMA_RUN);
            return (ret == LZMA_OK) ? b_[co_] : traits_type::eof();
        } else {
            return traits_type::eof();
        }
    }
}
ifbuf::int_type ifbuf::uflow() {
    auto next = underflow();
    if (next != traits_type::eof())
        ++co_;
    return next;
}
std::streamsize ifbuf::xsgetn(char_type* s, std::streamsize count) {
    std::streamsize gcount = 0;
    while (underflow() != traits_type::eof()) {
        auto rdcount =
            std::min(count, static_cast<std::streamsize>(avail_bytes()));
        std::copy_n(&b_[co_], rdcount, reinterpret_cast<uint8_t*>(s));
        co_ += rdcount;
        gcount += rdcount;
        if (gcount == count)
            break;
        s = &s[rdcount];
        count -= rdcount;
    }
    return gcount;
}
// end ifbuf implementation
// begin ofbuf implementation
ofbuf::ofbuf() noexcept
    : open_(false), fd_(-1), ls_(LZMA_STREAM_INIT), ib_(nullptr), ob_(nullptr),
      bs_(0) {}
ofbuf::~ofbuf() { close(); }

bool ofbuf::is_open() const noexcept { return open_; }
bool ofbuf::open(const char* f, uint32_t preset, bool mt, size_t bs) noexcept {
    if (open_)
        close();
    // create files with rw-r--r-- permissions
    fd_ = ::open(f, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_ == -1)
        return false;
    ls_ = LZMA_STREAM_INIT;
    if (mt) {
        lzma_mt mt;
        mt.flags = 0;
        mt.block_size = 0;
        mt.timeout = 0;
        mt.preset = preset;
        mt.filters = nullptr;
        mt.check = LZMA_CHECK_CRC64;
        mt.threads = std::thread::hardware_concurrency();
        auto ret = lzma_stream_encoder_mt(&ls_, &mt);
        if (ret != LZMA_OK)
            return false;
    } else {
        auto ret = lzma_easy_encoder(&ls_, preset, LZMA_CHECK_CRC64);
        if (ret != LZMA_OK)
            return false;
    }
    bs_ = bs;
    ib_ = std::make_unique<uint8_t[]>(bs_);
    ob_ = std::make_unique<uint8_t[]>(bs_);
    ls_.next_in = ib_.get();
    ls_.avail_in = 0;
    ls_.next_out = ob_.get();
    ls_.avail_out = bs_;
    open_ = true;
    return true;
}
void ofbuf::close() noexcept {
    if (!open_)
        return;
    for (;;) {
        auto ret = lzma_code(&ls_, LZMA_FINISH);
        if (!ls_.avail_out || (ret == LZMA_STREAM_END))
            sync();
        if (ret != LZMA_OK)
            break;
    }
    ::close(fd_);
    fd_ = -1;
    lzma_end(&ls_);
    open_ = false;
}
uint64_t ofbuf::progress() noexcept {
    if (!open_)
        return 0;
    uint64_t prog_in, prog_out;
    lzma_get_progress(&ls_, &prog_in, &prog_out);
    return prog_in;
}

int ofbuf::sync() {
    auto wrcount = bs_ - ls_.avail_out;
    if (!wrcount)
        return 0;
    auto wret = write(fd_, ob_.get(), wrcount);
    ls_.next_out = ob_.get();
    ls_.avail_out = bs_;
    return ((wret == -1) ? -1 : 0);
}
ofbuf::int_type ofbuf::overflow(int_type ch) {
    // check for need to compress
    if (ls_.avail_in == bs_) {
        while (ls_.avail_in) {
            auto ret = lzma_code(&ls_, LZMA_RUN);
            if (ret != LZMA_OK)
                return traits_type::eof();
            if (!ls_.avail_out && (sync() == -1))
                return traits_type::eof();
        }
        ls_.next_in = ib_.get();
        ls_.avail_in = 0;
    }
    // overflow() technically lets you add a character into the put area
    if (ch != traits_type::eof())
        ib_[ls_.avail_in++] = ch;
    return traits_type::to_int_type(0);
}
std::streamsize ofbuf::xsputn(const char_type* s, std::streamsize count) {
    std::streamsize pcount = 0;
    while (pcount != count) {
        if (overflow() == traits_type::eof())
            break;
        auto wrcount = std::min(
            count - pcount, static_cast<std::streamsize>(bs_ - ls_.avail_in));
        std::copy_n(&s[pcount], wrcount, &ib_[ls_.avail_in]);
        pcount += wrcount;
        ls_.avail_in += wrcount;
    }
    return pcount;
}
// end ofbuf implementation
// begin ifstream implementation
ifstream::ifstream() noexcept : std::istream(&ifb_), ifb_() {}
ifstream::ifstream(const char* f, size_t bs) noexcept
    : std::istream(&ifb_), ifb_() {
    open(f, bs);
}
ifstream::ifstream(const std::string& s, size_t bs) noexcept
    : std::istream(&ifb_), ifb_() {
    open(s.c_str(), bs);
}
#ifdef _CXX_HAS_FILESYSTEM
ifstream::ifstream(const std::filesystem::path& p, size_t bs) noexcept
    : std::istream(&ifb_), ifb_() {
    open(p.c_str(), bs);
}
#endif

bool ifstream::is_open() const noexcept { return ifb_.is_open(); }
void ifstream::open(const char* f, size_t bs) noexcept {
    if (!ifb_.open(f, bs))
        setstate(std::ios_base::failbit);
}
void ifstream::open(const std::string& s, size_t bs) noexcept {
    open(s.c_str(), bs);
}
#ifdef _CXX_HAS_FILESYSTEM
void ifstream::open(const std::filesystem::path& p, size_t bs) noexcept {
    open(p.c_str(), bs);
}
#endif
void ifstream::close() noexcept { ifb_.close(); }
uint64_t ifstream::progress() noexcept { return ifb_.progress(); }
// end ifstream implementation
// begin ofstream implementation
ofstream::ofstream() noexcept : std::ostream(&ofb_), ofb_() {}
ofstream::ofstream(const char* f, uint32_t preset, size_t bs) noexcept
    : std::ostream(&ofb_), ofb_() {
    open(f, preset, bs);
}
ofstream::ofstream(const std::string& s, uint32_t preset, size_t bs) noexcept
    : std::ostream(&ofb_), ofb_() {
    open(s.c_str(), preset, bs);
}
#ifdef _CXX_HAS_FILESYSTEM
ofstream::ofstream(const std::filesystem::path& p, uint32_t preset,
                   size_t bs) noexcept
    : std::ostream(&ofb_), ofb_() {
    open(p.c_str(), preset, bs);
}
#endif

bool ofstream::is_open() const noexcept { return ofb_.is_open(); }
void ofstream::open(const char* f, uint32_t preset, size_t bs) noexcept {
    if (!ofb_.open(f, preset, false, bs))
        ;
    setstate(std::ios_base::failbit);
}
void ofstream::open(const std::string& s, uint32_t preset, size_t bs) noexcept {
    open(s.c_str(), preset, bs);
}
#ifdef _CXX_HAS_FILESYSTEM
void ofstream::open(const std::filesystem::path& p, uint32_t preset,
                    size_t bs) noexcept {
    open(p.c_str(), preset, bs);
}
#endif
void ofstream::close() noexcept { ofb_.close(); }
uint64_t ofstream::progress() noexcept { return ofb_.progress(); }
// end ofstream implementation
// begin mtofstream implementation
mtofstream::mtofstream() noexcept : std::ostream(&ofb_), ofb_() {}
mtofstream::mtofstream(const char* f, uint32_t preset, size_t bs) noexcept
    : std::ostream(&ofb_), ofb_() {
    open(f, preset, bs);
}
mtofstream::mtofstream(const std::string& s, uint32_t preset,
                       size_t bs) noexcept
    : std::ostream(&ofb_), ofb_() {
    open(s.c_str(), preset, bs);
}
#ifdef _CXX_HAS_FILESYSTEM
mtofstream::mtofstream(const std::filesystem::path& p, uint32_t preset,
                       size_t bs) noexcept
    : std::ostream(&ofb_), ofb_() {
    open(p.c_str(), preset, bs);
}
#endif

bool mtofstream::is_open() const noexcept { return ofb_.is_open(); }
void mtofstream::open(const char* f, uint32_t preset, size_t bs) noexcept {
    if (!ofb_.open(f, preset, true, bs))
        setstate(std::ios_base::failbit);
}
void mtofstream::open(const std::string& s, uint32_t preset,
                      size_t bs) noexcept {
    open(s.c_str(), preset, bs);
}
#ifdef _CXX_HAS_FILESYSTEM
void mtofstream::open(const std::filesystem::path& p, uint32_t preset,
                      size_t bs) noexcept {
    open(p.c_str(), preset, bs);
}
#endif
void mtofstream::close() noexcept { ofb_.close(); }
uint64_t mtofstream::progress() noexcept { return ofb_.progress(); }
// end mtofstream implementation
} // namespace lzma
