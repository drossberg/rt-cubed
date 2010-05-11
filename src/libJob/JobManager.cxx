/*                  J O B M A N A G E R . C X X
 * BRL-CAD
 *
 * Copyright (c) 2010 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file JobManager.cxx
 *
 * Singleton.
 *
 */

#include "JobManager.h"
#include <QMutexLocker>
#include <iostream>

JobManager* JobManager::pInstance = NULL;

JobManager::JobManager()
{
    this->log = Logger::getInstance();
    this->jobQueue = new QList<AbstractJob*> ();
    this->queueLock = new QMutex();

    this->jobWorkers = new QList<JobWorker*> ();

    QString text = "Startup.  MAX_JOBWORKERS: " + QString::number(
	    MAX_JOBWORKERS);

    this->log->logINFO("JobManager", text);

    for (quint32 i = 0; i < MAX_JOBWORKERS; ++i) {
	JobWorker* jw = new JobWorker();
	this->jobWorkers->append(jw);
	jw->start();

	text = "Created new JobWorker with ID of " + jw->getWorkerIdAsQString();
	this->log->logINFO("JobManager", text);
    }

    text = "Created a total of " + QString::number(this->jobWorkers->size()) + " JobWorkers";
    this->log->logINFO("JobManager", text);
}

JobManager::~JobManager()
{
    delete jobQueue;
    //TODO Should I loop through jobs, destroying them as well?
    delete queueLock;

    //loop through workers
    while (!this->jobWorkers->isEmpty()) {
	JobWorker* jw = this->jobWorkers->front();

	jw->shutdown();

	this->jobWorkers->pop_front();
    }

    delete jobWorkers;
}

JobManager* JobManager::getInstance()
{
    if (!JobManager::pInstance) {
	pInstance = new JobManager();
    }
    return JobManager::pInstance;
}

void JobManager::submitJob(AbstractJob* job)
{
    QMutexLocker locker(this->queueLock);
    this->jobQueue->append(job);
}

AbstractJob* JobManager::getNextJob()
{
    QMutexLocker locker(this->queueLock);
    if (!this->jobQueue->isEmpty()) {
	return this->jobQueue->takeFirst();
    }
    else {
	return NULL;
    }
}

bool JobManager::hasJobsToWork()
{
    QMutexLocker locker(this->queueLock);
    return !this->jobQueue->isEmpty();
}

quint32 JobManager::getWorkQueueLen()
{
    return this->jobQueue->size();
}

//TODO add JobWorker Monitor.

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
