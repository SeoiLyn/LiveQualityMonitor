import web
from subprocess import Popen
from numpy import genfromtxt
import uuid
from threading import Timer
import ConfigParser, os

#define some default path and exe value
projects_root = "/home/username/projects/"
panel_root = projects_root+"LiveQualityPanel/"
brisque_path = projects_root + "LiveQualityMonitor/brisque_revised/"
monitor_exe = projects_root+'LiveQualityMonitor/debug/LiveQualityMonitor'
sever_port = 8080

urls = (
    '/', 'index',
	'/monitor', 'monitor'
)

render = web.template.render('templates/')

def readConfig():
    config = ConfigParser.ConfigParser()
    config.read('./config.cfg')

    global projects_root
    projects_root = config.get('path', 'projects_root')
    print "config projects root: "+projects_root

    global panel_root
    panel_root = config.get('path', 'panel_root')
    print "config panel root: "+panel_root

    global monitor_exe
    monitor_exe = config.get('path', 'monitor_exe')
    print "config monitor exe: "+monitor_exe

    global brisque_path
    brisque_path = config.get('path', 'brisque_path')
    print "config brisque path: "+brisque_path

    global sever_port
    sever_port = config.getint('server', 'port')
    print "config server port: "+str(sever_port)

def killMonitorProcess(*args):
    print "kill monitor process"
    for each in args:
        print each
        each.kill()

class index:
    def GET(self):
        return render.index()

class monitor:
    def POST(self):
        readConfig()

        i = web.input()
        flv_url = i['url']
        name = str(uuid.uuid4())
        record_read = "static/data/quality-"+ name +".csv"

        print panel_root
        print monitor_exe
        print brisque_path
        p = Popen([monitor_exe, flv_url, panel_root+record_read,brisque_path])

        t = Timer(600.0, killMonitorProcess, [p])
        t.start() # after 300 seconds, timer will trigger to stop the subprocess

        return render.monitor(record_read)
    def GET(self):
        user_data = web.input()
        print user_data.name
        return render.monitor(user_data.name)

class MyApplication(web.application):
    def run(self, port=8080, *middleware):
        func = self.wsgifunc(*middleware)
        return web.httpserver.runsimple(func, ('0.0.0.0', port))

if __name__ == "__main__":
    readConfig()

    app = MyApplication(urls, globals())
    app.run(port=sever_port)