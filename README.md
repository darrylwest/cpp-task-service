# Task Runner Service Application

```
 _______               __      _______                    __             
|_     _|.---.-.-----.|  |--. |     __|.-----.----.--.--.|__|.----.-----.
  |   |  |  _  |__ --||    <  |__     ||  -__|   _|  |  ||  ||  __|  -__|
  |___|  |___._|_____||__|__| |_______||_____|__|  \___/ |__||____|_____|
                                                                         
```

## Overview

A client/server service that triggers shell commands on client(s) when triggered by the server.  
A use case is when you have multiple servers that need to compile the same source when a file changes and a commit is made.

## Installation Instructions

* [ ] clone the [repo](https://github.com/darrylwest/cpp-task-service.git) 
* [ ] cd to cpp-task-service and run `./mk init build`
* [ ] install the service binary in /usr/local/bin/task-service
* [ ] run the service
* [ ] build the clients on remote dev machines
* [ ] navigate to the target you want to build
* [ ] use curl to post commands to the remote clients

## WARNING!

This app shoud only be run on secured networks.  Really bad things can happen through the shell.  Use this at your own risk!

###### 2024.12.23 | dpw
