
#include "../include/core.h"


int main(){

 core::on_init();

 while (!core::appDone()) {
    core::on_event();
    core::on_update();
    }

 core::on_quit();

    return 0;
}
