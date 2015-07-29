#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "LogUtil.h"

#define LOG_MSG_BUF_SIZE 2048

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/PatternLayout.hh>

log4cpp::Category *category;

char * logMsgBuf = NULL;
pthread_mutex_t mutux_log4cpp = PTHREAD_MUTEX_INITIALIZER;

char* getMsgBuf() {
	if(logMsgBuf == NULL) {
		logMsgBuf = (char*) malloc(LOG_MSG_BUF_SIZE);
		if(logMsgBuf == NULL) {
			category->error("logUtils: malloc fails");
			exit(-1);
		}

        log4cpp::Appender *appender = new log4cpp::FileAppender("FileAppender",LOG_FILE_PATH);
	//log4cpp::Layout* layout = new log4cpp::BasicLayout();
	log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
	layout->setConversionPattern("%d: %p - %m %n");
	appender->setLayout(layout);

	category = &log4cpp::Category::getInstance("Category");

        category->setAppender(appender);
        category->setPriority(log4cpp::Priority::DEBUG);
	}
	return logMsgBuf;
}

void verbose(const char * tag, const char * stringFormat, ...) {
	pthread_mutex_lock(&mutux_log4cpp);
	char * msgBuf = getMsgBuf();
	va_list va;
	va_start(va, stringFormat);
	vsnprintf(msgBuf, LOG_MSG_BUF_SIZE, stringFormat, va);
	category->log(log4cpp::Priority::NOTSET, msgBuf);
	va_end(va);
	pthread_mutex_unlock(&mutux_log4cpp);
}


void debug(const char * tag, const char * stringFormat, ...) {
	if(strcmp(tag, LOG_MISC) == 0) return;

	pthread_mutex_lock(&mutux_log4cpp);
	char * msgBuf = getMsgBuf();
	va_list va;
	va_start(va, stringFormat);
	vsnprintf(msgBuf, LOG_MSG_BUF_SIZE, stringFormat, va);
	category->debug(msgBuf);
	va_end(va);
	pthread_mutex_unlock(&mutux_log4cpp);
}

void info(const char * tag, const char * stringFormat, ...) {
	pthread_mutex_lock(&mutux_log4cpp);
	char * msgBuf = getMsgBuf();
	// check out http://en.wikipedia.org/wiki/Stdarg.h
	// for the manipulation of variable number of arguments
    va_list va;
    va_start(va, stringFormat);
    vsnprintf(msgBuf, LOG_MSG_BUF_SIZE, stringFormat, va);
    category->info(msgBuf);
    va_end(va);
    pthread_mutex_unlock(&mutux_log4cpp);
}

void warn(const char * tag, const char * stringFormat, ...) {
	pthread_mutex_lock(&mutux_log4cpp);
	char * msgBuf = getMsgBuf();
	va_list va;
	va_start(va, stringFormat);
	vsnprintf(msgBuf, LOG_MSG_BUF_SIZE, stringFormat, va);
	category->warn(msgBuf);
	va_end(va);
	pthread_mutex_unlock(&mutux_log4cpp);
}

void error(const char * tag, const char * stringFormat, ...) {
	pthread_mutex_lock(&mutux_log4cpp);
	char * msgBuf = getMsgBuf();
	va_list va;
	va_start(va, stringFormat);
	vsnprintf(msgBuf, LOG_MSG_BUF_SIZE, stringFormat, va);
	category->error(msgBuf);
	va_end(va);
	pthread_mutex_unlock(&mutux_log4cpp);
}

