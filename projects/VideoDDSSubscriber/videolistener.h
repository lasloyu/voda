// Copyright 2017 S2E Software, Systems and Control 
//  
// Licensed under the Apache License, Version 2.0 the "License"; 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//  
//    http://www.apache.org/licenses/LICENSE-2.0 
//  
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.

#ifndef VIDEOLISTENER_H
#define VIDEOLISTENER_H

#include <QObject>
#include <gst/app/gstappsrc.h>

#include "VideoDDS_DCPS.hpp"

/**
 * This class is a DDS Data Reader Listener and can be used to listen to
 * S2E::Video with a subscriber.
 *
 * It uses GStreamer functions to push data that is received into an
 * Gstreamer appsrc element. This element must be set first with installAppSrc().
 *
 * Note: it inherits the NoOpDataReaderListener to have no operation methods
 * implemented for the pure virtual methods from DataReaderListener.
 * This is according the OpenSplice ExampleDataReaderListener.
 */
class VideoListener : public virtual dds::sub::DataReaderListener<S2E::Video>,
		public virtual dds::sub::NoOpDataReaderListener<S2E::Video>
{
public:
	VideoListener();

	/**
	 * Set the appsrc element that is used by on_data_available() to
	 * push arrived data into.
	 */
	void installAppSrc(GstAppSrc* appSrc);

	/**
	 * Only prints an info that the class is destructed.
	 */
	virtual ~VideoListener();

	/**
	 * Only prints an info that a deadline was missed.
	 */
	virtual void on_requested_deadline_missed(
		dds::sub::DataReader<S2E::Video>& the_reader,
		const dds::core::status::RequestedDeadlineMissedStatus& status);

	/**
	 * Takes data from the DDS system and copies it into the GStreamer appsrc.
	 */
	virtual void on_data_available(dds::sub::DataReader<S2E::Video>& reader);

protected:

	/**
	 * Should be used in the future to enable copy-free data handling.
	 */
	static void gstBufferDestroyCallBack(gpointer data);

private:
	GstAppSrc* m_appSrc;
};

#endif // VIDEOLISTENER_H
