from smartsprinkler import SmartSprinkler
import datetime


if __name__=="__main__":
    time = datetime.datetime.now().time()
    date = datetime.datetime.now().date()

    con = SmartSprinkler("192.168.0.157")
    con.setDate([time.hour,time.minute,time.second],\
            [date.day,date.month,date.year-2000])

