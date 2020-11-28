#include <env.h>
#include <pmap.h>
#include <printf.h>
extern int debug_mode;
/* Overview:
 *  Implement simple round-robin scheduling.
 *  Search through 'envs' for a runnable environment ,
 *  in circular fashion statrting after the previously running env,
 *  and switch to the first such environment found.
 *
 * Hints:
 *  The variable which is for counting should be defined as 'static'.
 */
void sched_yield(void)
{
	static int time_counter = 0;
	static struct Env * cur = NULL;
	static int index = 0;
	while(1) {
		while(LIST_FIRST(&env_sched_list[index])==NULL) {
			index = 1 - index;
		}
		cur = LIST_FIRST(&env_sched_list[index]);
		// time_counter = cur->env_pri;
		if(debug_mode) {
//			printf("[DEBUG] env: %d \n", cur!=NULL);	
//			printf("[DEBUG] env_pri: %d\n",cur->env_pri);
//			printf("[DEBUG] env_status:%d\n",cur->env_status == ENV_RUNNABLE);
	
		}
		if(time_counter >= cur->env_pri || cur->env_status != ENV_RUNNABLE) {
			time_counter = 0;
			LIST_REMOVE(cur, env_sched_link);
			LIST_INSERT_HEAD(&env_sched_list[1-index], cur, env_sched_link);
		}
		else {
			time_counter++;
			break;
		}
	}
	env_run(cur);
}
