#ifndef THREAD_H
#define THREAD_H (1)

#define STACK_SIZE (4096)

/*! \brief starts a new thread in the calling process
 * \NOTE: thread: a control flow with separate PC, registers and stack
 *         and same DS (thus static and globals), files, code
 *
 * \param[in]: 
 * thread - ID of the newly created thread is stored in buffer pointed by thread
 * start_routine - function from where the new thread will start execution
 * arg - argument to start_routine
 *
 * \return: On success, 0. Else errno
 */
int thread_create(int *thread, int (*start_routine)(void *), void *arg);

/*! \brief Waits for the specified thread to terminate
 *
 * \NOTE: The specified thread must be joinable
 *
 * \param thread: thread ID of thread to be joined
 * \return: 0 on success, errno on error
 */
int thread_join(int thread);

/*! \brief Terminates a calling thread
 *
 * \param retval: return value of function is pointed by this parameter
 * \return does not return
 */
void thread_exit(void *retval);

#endif /* THREAD_H */
