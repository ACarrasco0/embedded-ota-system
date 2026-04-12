
#include "app/config.h"
#include "modules/logger/logger.h"
#include "system/system.h"
#include "system/signals.h"

int main() {
	
	setup_signals();
	system_init();
	
	log_info("App working again ! ");
	
	while(app_running){
		// Main loop of the application
		system_update();

		usleep(10000); 
	}

	system_deinit();
	log_info("App disconnected ! ");

	return 0;
} 	
