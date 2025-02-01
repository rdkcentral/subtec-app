##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service B.V.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################

import dbus
import json

def teletext_dbus_test():
    
    bus_name = 'com.libertyglobal.rdk.teletext'
    object_path = '/com/libertyglobal/rdk/teletext'
    my_dbus_interface = 'com.libertyglobal.rdk.teletext'
    
    bus = dbus.SessionBus()
    proxy = bus.get_object(bus_name, object_path)

    print

    if (proxy != None):
        print("Got Teletext proxy: " + str(proxy))

    introspection_interface = dbus.Interface(proxy, dbus.INTROSPECTABLE_IFACE)
    
    # Introspectable interfaces define a property 'Introspect' that
    # will return an XML string that describes the object's interface
    interface = introspection_interface.Introspect()
    print "### Introspection data:"
    print interface

    print
    print "### Teletext status:"

    getStatusResponse = proxy.getStatus(dbus_interface=my_dbus_interface)
    print getStatusResponse

    print "### setting muted to true:"
    
    proxy.setMuted(json.dumps({"muted": True}), dbus_interface=my_dbus_interface)

    print "### Teletext status:"

    getStatusResponse = proxy.getStatus(dbus_interface=my_dbus_interface)
    print getStatusResponse
    
    jsonResponse = json.loads(getStatusResponse)
        
    if jsonResponse["muted"] != True:
        print "ERROR: incorrect muted status"

    print "### setting muted to false:"

    proxy.setMuted(json.dumps({"muted": False}), dbus_interface=my_dbus_interface)

    print "### Teletext status:"
    
    getStatusResponse = proxy.getStatus(dbus_interface=my_dbus_interface)
    print getStatusResponse
    
    jsonResponse = json.loads(getStatusResponse)
        
    if jsonResponse["muted"] != False:
        print "ERROR: incorrect muted status"

def subtitle_dbus_test():

    bus_name = 'com.libertyglobal.rdk.subtitles'
    object_path = '/com/libertyglobal/rdk/subtitles'
    my_dbus_interface = 'com.libertyglobal.rdk.subtitles'
    
    bus = dbus.SessionBus()
    proxy = bus.get_object(bus_name, object_path)

    print
    if (proxy != None):
        print("Got Subtitles proxy: " + str(proxy))
        
    introspection_interface = dbus.Interface(proxy, dbus.INTROSPECTABLE_IFACE)
    
    # Introspectable interfaces define a property 'Introspect' that
    # will return an XML string that describes the object's interface
    interface = introspection_interface.Introspect()
    print "### Introspection data:"
    print interface
    
    print
    print "### Subtitles status:"

    getStatusResponse = proxy.getStatus(dbus_interface=my_dbus_interface)
    print getStatusResponse

    print "### setting muted to true:"
    
    proxy.setMuted(json.dumps({"muted": True}), dbus_interface=my_dbus_interface)

    print "### Subtitles status:"

    getStatusResponse = proxy.getStatus(dbus_interface=my_dbus_interface)
    print getStatusResponse
    
    jsonResponse = json.loads(getStatusResponse)
        
    if jsonResponse["muted"] != True:
        print "ERROR: incorrect muted status"

    print "### setting muted to false:"

    proxy.setMuted(json.dumps({"muted": False}), dbus_interface=my_dbus_interface)

    print "### Subtitles status:"
    
    getStatusResponse = proxy.getStatus(dbus_interface=my_dbus_interface)
    print getStatusResponse
    
    jsonResponse = json.loads(getStatusResponse)
        
    if jsonResponse["muted"] != False:
        print "ERROR: incorrect muted status"

def subttxrend_dbus_tests():
    subtitle_dbus_test()
    teletext_dbus_test()

if __name__ == '__main__':
    subttxrend_dbus_tests()
