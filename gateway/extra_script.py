Import("env")


def before_upload(source, target, env):
    print("Pre action")
    # env.Execute("esptool.exe erase_flash")
    env.Execute("esptool.exe read_mac > mac.txt")
    mac = ""
    with open('mac.txt', 'r') as file:
        lines = file.readlines()
        for line in lines:
            data = line.split(' ')
            if data[0] == 'MAC:':
                mac = data[1].upper()
                break
    hasMac = False
    with open('hosts.txt', 'r+') as file:
        lineNumber = 0
        # print(mac.upper())
        lines = file.readlines()
        # print(lines)
        for line in lines:
            if line == mac:
                hasMac = True
                break
        if not hasMac:
            # print(hasMac)
            file.write(mac)
    env.Execute('del mac.txt')


env.AddPreAction("upload", before_upload)
