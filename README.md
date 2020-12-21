# process-trace-url-extend
Purpose: System call interception to access remote URLs as if they were local files

How to use: Type make to compile the binary and object files. ./urlextend <application with flags> <webpage>

Purpose: Be able to run Linux shell commands on remote URLs. These include less, wc, cat, etc.

Urlextend intercepts system calls to analyze appplications' use and provide information in one easy step. It uses fork/execvp calls to wget(1) and whichever terminal application the user decides to use.
