.. _semaphores_v2:

Semaphores
##########

A :dfn:`semaphore` is a kernel object that implements a traditional
counting semaphore.

.. contents::
    :local:
    :depth: 2

Concepts
********

Any number of semaphores can be defined. Each semaphore is referenced
by its memory address.

A semaphore has the following key properties:

* A **count** that indicates the number of times the semaphore can be taken.
  A count of zero indicates that the semaphore is unavailable.

* A **limit** that indicates the maximum value the semaphore's count
  can reach.

A semaphore must be initialized before it can be used. Its count must be set
to a non-negative value that is less than or equal to its limit.

A semaphore may be **given** by a thread or an ISR. Giving the semaphore
increments its count, unless the count is already equal to the limit.

A semaphore may be **taken** by a thread. Taking the semaphore
decrements its count, unless the semaphore is unavailable (i.e. at zero).
When a semaphore is unavailable a thread may choose to wait for it to be given.
Any number of threads may wait on an unavailable semaphore simultaneously.
When the semaphore is given, it is taken by the highest priority thread
that has waited longest.

.. note::
    The kernel does allow an ISR to take a semaphore, however the ISR must
    not attempt to wait if the semaphore is unavailable.

Implementation
**************

Defining a Semaphore
====================

A semaphore is defined using a variable of type :c:type:`struct k_sem`.
It must then be initialized by calling :cpp:func:`k_sem_init()`.

The following code defines a semaphore, then configures it as a binary
semaphore by setting its count to 0 and its limit to 1.

.. code-block:: c

    struct k_sem my_sem;

    k_sem_init(&my_sem, 0, 1);

Alternatively, a semaphore can be defined and initialized at compile time
by calling :c:macro:`K_SEM_DEFINE()`.

The following code has the same effect as the code segment above.

.. code-block:: c

    K_SEM_DEFINE(my_sem, 0, 1);

Giving a Semaphore
==================

A semaphore is given by calling :cpp:func:`k_sem_give()`.

The following code builds on the example above, and gives the semaphore to
indicate that a unit of data is available for processing by a consumer thread.

.. code-block:: c

    void input_data_interrupt_handler(void *arg)
    {
        /* notify thread that data is available */
        k_sem_give(&my_sem);

        ...
    }

Taking a Semaphore
==================

A semaphore is taken by calling :cpp:func:`k_sem_take()`.

The following code builds on the example above, and waits up to 50 milliseconds
for the semaphore to be given.
A warning is issued if the semaphore is not obtained in time.

.. code-block:: c

    void consumer_thread(void)
    {
        ...

        if (k_sem_take(&my_sem, 50) != 0) {
            printk("Input data not available!");
        } else {
            /* fetch available data */
            ...
        }
        ...
    }

Suggested Uses
**************

Use a semaphore to control access to a set of resources by multiple threads.

Use a semaphore to synchronize processing between a producing and consuming
threads or ISRs.

Configuration Options
*********************

Related configuration options:

* None.

APIs
****

The following semaphore APIs are provided by :file:`kernel.h`:

* :cpp:func:`k_sem_init()`
* :cpp:func:`k_sem_give()`
* :cpp:func:`k_sem_take()`
* :cpp:func:`k_sem_reset()`
* :cpp:func:`k_sem_count_get()`
