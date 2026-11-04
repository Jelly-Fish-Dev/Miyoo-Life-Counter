/* Life Counter – MTG-style life counter for the Miyoo Mini / Mini Plus. */

#include "app/app.h"
#include "platform/log.h"
#include "version.h"

int main(void)
{
    LOG_INFO("Life Counter %s starting", LC_VERSION);
    return app_run();
}
