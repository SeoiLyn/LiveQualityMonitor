import web
from subprocess import Popen
from numpy import genfromtxt
import uuid

record_root = "/home/water/projects/LiveQualityPanel/"

urls = (
    '/', 'index',
	'/monitor', 'monitor'
)

render = web.template.render('templates/')

class index:
    def GET(self):
        return render.index()

class monitor:
    def POST(self):
        i = web.input()
        flv_url = i['url']
        name = str(uuid.uuid4())
        record_read = "static/data/quality-"+ name +".csv"
        p = Popen(['/home/water/projects/LiveQualityMonitor/debug/LiveQualityMonitor', flv_url, record_root+record_read])
        return render.monitor(record_read)
    def GET(self):
        user_data = web.input()
        #return "<h1>" + user_data.name + "</h1>"
        print user_data.name
        return render.monitor(user_data.name)

if __name__ == "__main__":
    app = web.application(urls, globals())
    app.run()