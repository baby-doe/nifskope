/***** BEGIN LICENSE BLOCK *****

BSD License

Copyright (c) 2005, NIF File Format Library and Tools
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the NIF File Format Library and Tools projectmay not be
   used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***** END LICENCE BLOCK *****/

#ifndef GLCONTROLLER_H
#define GLCONTROLLER_H

#include "nifmodel.h"

#include <QPointer>

class Scene;
class Node;
class Controller;
class Interpolator;

class Controller
{
	typedef union
	{
		quint16 bits;
		
		struct Controller
		{
			bool unknown : 1;
			enum
			{
				Cyclic = 0, Reverse = 1, Constant = 2
			} extrapolation : 2;
			bool active : 1;
		} controller;
		
	} ControllerFlags;
	
public:
	Controller( const QModelIndex & index );
	virtual ~Controller() {}
	
	float start;
	float stop;
	float phase;
	float frequency;
	
	ControllerFlags flags;
	
	virtual void update( float time ) = 0;
	
	virtual bool update( const NifModel * nif, const QModelIndex & index );
	
	QModelIndex index() const { return iBlock; }
	
	float ctrlTime( float time ) const;
	
	template <typename T> static bool interpolate( T & value, const QModelIndex & data, const QString & arrayid, float time, int & lastindex );
	template <typename T> static bool interpolate( T & value, const QModelIndex & array, float time, int & lastIndex );	
	static bool timeIndex( float time, const NifModel * nif, const QModelIndex & array, int & i, int & j, float & x );
	
protected:
   friend class Interpolator;

	QPersistentModelIndex iBlock;
	QPersistentModelIndex iInterpolator;
	QPersistentModelIndex iData;
};

template <typename T> bool Controller::interpolate( T & value, const QModelIndex & data, const QString & arrayid, float time, int & lastindex )
{
	const NifModel * nif = static_cast<const NifModel *>( data.model() );
	if ( nif && data.isValid() )
	{
		QModelIndex array = nif->getIndex( data, arrayid );
		return interpolate( value, array, time, lastindex );
	}
	else
		return false;
}

class Interpolator : public QObject
{
public:
   Interpolator(Controller *owner);

   virtual bool update( const NifModel * nif, const QModelIndex & index );

protected:
   QPersistentModelIndex& GetControllerData();
   Controller *parent;
};

class TransformInterpolator : public Interpolator
{
public:
   TransformInterpolator(Controller *owner);

   virtual bool update( const NifModel * nif, const QModelIndex & index );
   virtual bool updateTransform(Transform& tm, float time);

protected:
   QPersistentModelIndex iTranslations, iRotations, iScales;
   int lTrans, lRotate, lScale;
};

class BSplineTransformInterpolator : public TransformInterpolator
{
public:
   BSplineTransformInterpolator( Controller *owner );

   virtual bool update( const NifModel * nif, const QModelIndex & index );
   virtual bool updateTransform(Transform& tm, float time);

protected:
   float start, stop;
   QPersistentModelIndex iControl, iSpline, iBasis;
   QPersistentModelIndex lTrans, lRotate, lScale;
   ushort lTransOff, lRotateOff, lScaleOff;
   float lTransMult, lRotateMult, lScaleMult;
   float lTransBias, lRotateBias, lScaleBias;
   uint nCtrl;
   int degree;
};


#endif

