#include <ThING/core.h>

void ThING::detail::setResizedFlag(ProtoThiApp& app, bool flag){
    app.framebufferResized = true;
}