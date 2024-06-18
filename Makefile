# Example Makefile for the exam task + exam submission zip file creation.
CFLAGS = -std=gnu11 -Wall -Wextra 
# potential other flags: -lpthread -D_DEFAULT_SOURCE
CC = gcc

.PHONY: exam_task all clean zip clean_for_zip

all: exam_task

exam_task: exam_task.c
	$(CC) $(CFLAGS) -o exam_task exam_task.c

clean:
	rm -f exam_task

# Prepare zip file for exam submission. Should normally not be in the same makefile that compiles the task, this is just for the exam.

EXCLUDE_PATTERNS = "**.vscode/*" "**.idea/*" "**__MACOSX/*" "**.DS_Store/*" "**.dSYM/*"
ARCHIVE= "../exam_csaz9096.zip"

zip: clean_for_zip
	$(RM) $(ARCHIVE)
	zip -r $(ARCHIVE) . --exclude $(EXCLUDE_PATTERNS)

clean_for_zip:
	$(MAKE) clean;
