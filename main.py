#!/opt/homebrew/bin/python3

import psutil

def main():
    cs_proc = next(p for p in list(psutil.process_iter()) if p.name().lower() == 'csgo_osx64').pid
    print(cs_proc)

if __name__ == '__main__':
    main()
