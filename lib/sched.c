#include <env.h>
#include <pmap.h>
#include <printf.h>

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
		if(LIST_FIRST(&env_sched_list[index])==NULL) {
			index = 1 - index;
		}
		cur = LIST_FIRST(&env_sched_list[index]);
		// time_counter = cur->env_pri;
		// printf("[DEBUG] if env null: %d \n", cur==NULL);	
		if(time_counter >= cur->env_pri) {
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
