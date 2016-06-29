import wx
import threading
import time
from smartsprinkler import SmartSprinkler
from wx.lib.pubsub import setuparg1
from wx.lib.pubsub import pub


class MainFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, title="Smart Sprinkler")


        self.comthread = ComThread()
        self.comthread.start()

        self.Bind(wx.EVT_CLOSE, self.on_close)
        self.pump_label = wx.StaticText(self, label="Pump control: ")
        self.pump_button = wx.ToggleButton(self, -1, "ON/OFF")
        self.Bind(wx.EVT_TOGGLEBUTTON, self.pumpCtrl, self.pump_button)


        self.themperature_label = wx.StaticText(self, label="Themperature")
        self.themperature_gauge = wx.Gauge(self, style=wx.GA_VERTICAL)
        self.humidity_label = wx.StaticText(self, label="Humidity")
        self.humidity_gauge = wx.Gauge(self, style=wx.GA_VERTICAL)
        self.light_label = wx.StaticText(self, label="Light")
        self.light_gauge = wx.Gauge(self, style=wx.GA_VERTICAL)

        grid = wx.GridBagSizer(hgap=10, vgap=10)
        grid.Add(self.pump_label, pos=(0,0))
        grid.Add(self.pump_button, pos=(0,1))

        grid.Add(self.themperature_label, pos=(1,0), flag = wx.ALIGN_CENTER)
        grid.Add(self.themperature_gauge, pos=(2,0), flag = wx.ALIGN_CENTER)

        grid.Add(self.humidity_label, pos=(1,1), flag = wx.ALIGN_CENTER)
        grid.Add(self.humidity_gauge, pos=(2,1), flag = wx.ALIGN_CENTER )

        grid.Add(self.light_label, pos=(1,2), flag = wx.ALIGN_CENTER)
        grid.Add(self.light_gauge, pos=(2,2), flag = wx.ALIGN_CENTER)

        main_sizer = wx.BoxSizer(wx.HORIZONTAL)
        main_sizer.Add(grid, 0, wx.ALL, border=10)
        self.SetSizerAndFit(main_sizer)

        # Create publisher receiver
        pub.subscribe(self.update, "refresh")
    
    def pumpCtrl(self, event):
         button = event.GetEventObject()
         self.comthread.pumpState = button.GetValue()

    def update(self, msg):
        self.themperature_gauge.SetRange(50)
        self.themperature_gauge.SetValue(self.comthread.envState[0])
        self.themperature_gauge.SetForegroundColour(wx.Colour(0,0,255))

        self.humidity_gauge.SetRange(255)
        self.humidity_gauge.SetValue(self.comthread.envState[1])
        self.humidity_gauge.SetForegroundColour(wx.Colour(0,0,255))

        self.light_gauge.SetRange(255)
        self.light_gauge.SetValue(self.comthread.envState[2])
        self.light_gauge.SetForegroundColour(wx.Colour(0,0,255))


    def on_close(self, event):
        dlg = wx.MessageDialog(
            self,
            "Do you really want to close this application?",
            "Confirm Exit", wx.OK | wx.CANCEL | wx.ICON_QUESTION)
        result = dlg.ShowModal()
        dlg.Destroy()
        if result == wx.ID_OK:
            self.comthread.running = False
            self.Destroy()


class AdcPage(wx.Panel):
    def __init__(self, parent, frame):
        wx.Panel.__init__(self, parent)


class ComThread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.running = True
        self.pumpState = False

    def run(self):
        while self.running:
            smartsprinkler = SmartSprinkler("192.168.0.201")
            if self.pumpState:
                self.envState = smartsprinkler.setOn()
            else:
                self.envState = smartsprinkler.setOff()
            smartsprinkler.close()
            wx.CallAfter(pub.sendMessage, "refresh")
            time.sleep(1)


class MyApp(wx.App):
    def OnInit(self):
        return True


    def on_close(self, event):
        dlg = wx.MessageDialog(
            self,
            "Do you really want to close this application?",
            "Confirm Exit", wx.OK | wx.CANCEL | wx.ICON_QUESTION)
        result = dlg.ShowModal()
        dlg.Destroy()
        if result == wx.ID_OK:
            self.comthread.running = False
            self.Destroy()


def main():
    app = MyApp(False)
    global frame
    frame = MainFrame()
    frame.Centre()
    frame.Show()
    app.MainLoop()


if __name__ == "__main__":
    main()
