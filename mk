#!/usr/bin/env bash
#
# dpw@plaza.localdomain | 2024-12-13 03:28:43
#
#

set -eu

export root=`pwd`

port=2032

export os="$(uname -s)"
export arch="$(uname -m)"

[ $os == "Linux" ] && {
    # export CC=/usr/bin/clang
    # export CXX=/usr/bin/clang++
    export CC=/usr/local/bin/gcc
    export CXX=/usr/local/bin/g++
    export FLAGS="-j8"
} || {
    export FLAGS="-j20"
}

# parse the cli
while [[ $# -gt 0 ]]
do
    case $1 in
        init)
            [ -d build ] || mkdir build
            (cd build && cmake ..)

            shift
        ;;
        build)
            clear

            (cd build && time make $FLAGS)
            $root/build/task-service --version

            shift
        ;;
        unit)
            $root/build/task-unit

            shift
        ;;
        test)
            $root/build/task-unit
            $root/build/integration
            bat --paging=never $root/service.log

            shift
        ;;
        run)
            # TODO check that task-service has been built and is newer that all the souces
            $root/build/task-service

            shift
        ;;
        client)
            $root/build/task-client

            exit 0
        ;;
        run-debug)
            # TODO check that task-service has been built and is newer that all the souces
            $root/build/task-service --base $root/html --level 0

            shift
        ;;
        format)
            clang-format -i include/taskservice/*.hpp src/*.cpp
            git status -s

            shift
        ;;
        clean)
            (cd build && make clean)
        
            shift
        ;;
        clobber)
            /bin/rm -fr build/

            shift
        ;;
        watch)
            watchexec --timings -c -w src/ -w include/ -e h,hpp,cpp ./mk build unit

            exit 0
        ;;

        shutdown)
            curl -k -XDELETE https://localhost:$port/shutdown

            shift
        ;;

        page|show)
            curl -k https://localhost:$port

            shift
        ;;
        pull)
            git pull

            shift
        ;;
        version)
            curl -k https://localhost:$port/version

            shift
        ;;


        help)
            echo "Targets:"
            echo ""
            echo "   init     : run the cmake command to create the build folder"
            echo "   build    : compile task-service to the build folder"
            echo "   test     : run all tests"
            echo "   run      : runs the app and shows version"
            echo "   run-debug: runs the service with logging set to debug"
            echo "   client   : runs the task client"
            echo "   format   : runs clang-format over includes and src"
            echo "   watch    : run watcher over source and include"
            echo "   clean    : remove binary builds but leave the build folder"
            echo "   clobber  : remove the entire build folder"
            echo "   pull     : pull the latest changes from git repo"
            echo "   show     : runs curl against localhost to view index page"
            echo "   shutdown : runs localhost curl shutdown the server"
            echo "   help     : show this help"
            
            exit 0
        ;;

        build)
            cd src && pwd && make && make unit

            shift
        ;;
        *) # the default
            ./mk help
            exit 0
        ;;

    esac
done
