#include "WsTest.h"
#include "TestLoop.h"

#include <string.h>

WsTest::WsTest(TestLoop* loop, long conn_id)
: loop_(loop)
, ws_(loop->eventLoop())
, conn_id_(conn_id)
{
    ws_.setWriteCallback([this] (KMError err) { onSend(err); });
    ws_.setErrorCallback([this] (KMError err) { onClose(err); });
    ws_.setDataCallback([this] (KMBuffer &buf, bool is_text, bool fin) {
        onData(buf);
    });
}

KMError WsTest::attachFd(SOCKET_FD fd, uint32_t ssl_flags, const KMBuffer *init_buf)
{
    ws_.setSslFlags(ssl_flags);
    return ws_.attachFd(fd, init_buf);
}

KMError WsTest::attachSocket(TcpSocket&& tcp, HttpParser&& parser, const KMBuffer *init_buf)
{
    return ws_.attachSocket(std::move(tcp), std::move(parser), init_buf);
}

int WsTest::close()
{
    ws_.close();
    return 0;
}

void WsTest::onSend(KMError err)
{
    //sendTestData();
}

void WsTest::onClose(KMError err)
{
    printf("WsTest::onClose, err=%d\n", err);
    ws_.close();
    loop_->removeObject(conn_id_);
}

void WsTest::onData(KMBuffer &buf)
{
    //printf("WsTest::onData, len=%u\n", len);
    int ret = ws_.send(buf, false);
    if(ret < 0) {
        ws_.close();
        loop_->removeObject(conn_id_);
    }// else should buffer remain data if ret < len
}

void WsTest::sendTestData()
{
    const size_t kBufferSize = 128*1024;
    uint8_t buf[kBufferSize] = {0};
    memset(buf, 'a', sizeof(buf));
    
    KMBuffer buf1(buf, kBufferSize/4, kBufferSize/4);
    KMBuffer buf2(buf + kBufferSize/4, kBufferSize/4, kBufferSize/4);
    KMBuffer buf3(buf + kBufferSize*2/4, kBufferSize/4, kBufferSize/4);
    KMBuffer buf4(buf + kBufferSize*3/4, kBufferSize/4, kBufferSize/4);
    buf1.append(&buf2);
    buf1.append(&buf3);
    buf1.append(&buf4);
    
    while (true) {
        int ret = ws_.send(buf1, false);
        if (ret < 0) {
            break;
        } else if (ret < sizeof(buf)) {
            // should buffer remain data if ret < sizeof(buf)
            //printf("WsTest::sendTestData, break, ret=%d\n", ret);
            break;
        }
    }
}
