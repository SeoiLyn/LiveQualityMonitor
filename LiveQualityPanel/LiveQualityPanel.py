import web
from subprocess import Popen
from numpy import genfromtxt

record_root = "/home/water/projects/LiveQualityPanel/"
record_read = "static/data/receive-001.csv"

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
		p = Popen(['/home/water/projects/LiveQualityMonitor/debug/LiveQualityMonitor', flv_url, record_root+record_read])
		return render.monitor(record_read)

if __name__ == "__main__":
    app = web.application(urls, globals())
    app.run()