/*
 * Copyright 2012 Facebook
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <Foundation/Foundation.h>

/*!
 @protocol
 
 @abstract
 Base protocol to enable typed access to graph object data
 
 @discussion
 The FBGraphObject protocol is the core type used by the Facebook iOS SDK to 
 represent objects in the Facebook Social Graph and the Facebook Open Graph (OG).
 The FBGraphObject class implements useful default functionality, but is rarely
 used directly by applications. The FBGraphObject protocol, in contrast is the
 base protocol for all graph object access via the Facebook iOS SDK. 
 
 Goals of the FBGraphObject types:
 <ul>
   <li> Lightweight/maintainable/robust </li>
   <li> Extensible and resilient to change, both by Facebook and third party (OG) </li>
   <li> Simple and natural extension to objective-c </li> 
 </ul>

 The FBGraphObject at its core is a duck typed (if it walks/swims/quacks... 
 its a duck) model which supports an optional static facade. Duck-typing achieves
 the flexibility necessary for Social Graph and OG uses, and the static facade
 increases discoverability, maintainability, robustness and simplicity.
 The following excerpt from the PlacesPickerSample shows a simple use of the 
 a facade protocol FBGraphPlace by an application: 
 <pre>
 @textblock
   - (void)placesPicker:(FBPlacesPickerView*)placesPicker
           didPickPlace:(NSDictionary<FBGraphPlace>*)place {
       // we'll use logging to show the simple typed property access to place and location info
       NSLog(@"place=%@, city=%@, state=%@, lat long=%@ %@", 
             place.name,
             place.location.city,
             place.location.state,
             place.location.latitude,
             place.location.longitude);
       ...
   }
 @/textblock
 </pre>
 
 Note that in this example, access to common place information is available through typed property
 syntax. But if at some point places in the Social Graph supported additional fields "foo" and "bar", not
 reflected in the FBGraphPlace protocol, the application could still access the values like so:
 <pre>
 @textblock
       NSString *foo = [place objectForKey:@"foo"]; // perhaps located at the ... in the preceding example
       NSNumber *bar = [place objectForKey:@"bar"]; // extensibility applies to Social and Open graph uses
 @/textblock
 </pre>
 
 In addition to untyped access, applications and future revisions of the SDK may add facade protocols by 
 declaring a protocol inheriting the FBGraphObject protocol, like so:
 <pre>
 @textblock
   @protocol MyGraphThing<FBGraphObject>
   @property (copy, nonatomic) NSString *id;
   @property (copy, nonatomic) NSString *name;
   @end
 @/textblock
 </pre>

 Important: facade implementations are inferred by graph objects returned by the methods of the SDK. This 
 means that no explicit implementation is required by application or SDK code. Any FBGraphObject instance 
 may be cast to any FBGraphObject facade protocol, and accessed via properties. If a field is not present 
 for a given facade property, the property will return nil.

 The following layer diagram depicts some of the concepts discussed thus far:
 <pre>
 @textblock
                      *-------------* *------------* *-------------**--------------------------*
           Facade --> | FBGraphUser | |FBGraphPlace| | MyGraphThing|| MyGraphPersonExtentension| ...
                      *-------------* *------------* *-------------**--------------------------*
                      *-----------------------------------------* *---------------------------------*
 Transparent impl --> | FBGraphObject<FBGraphObject> (instaces) | |   CustomClass<FBGraphObject>    |
                      *-----------------------------------------* *---------------------------------*
                      *-------------------**------------------------* *-----------------------------*
    Apparent impl --> |NSMutableDictionary||FBGraphObject (protocol)| |FBGraphObject (class methods)|
                      *-------------------**------------------------* *-----------------------------*
 @/textblock
 </pre>
 
 The *Facade* layer is meant for typed access to graph objects. The *Transparent impl* layer (more 
 specifically, the instance capabilities of FBGraphObject) are used by the SDK and app logic
 internally, but are not part of the public interface between application and SDK. The *Apparent impl*
 layer represents the lower-level "duck-typed" use of graph objects.

 Implementation note: the SDK returns NSMutableDictionary derived instances with types declared like
 one of the following:
 <pre>
 @textblock
   NSMutableDictionary<FBGraphObject> *obj;     // no facade specified (still castable by app)
   NSMutableDictionary<FBGraphPlace> *person;   // facade specified when possible
 @/textblock
 </pre>
 However, when passing a graph object to the SDK, NSMutableDictionary is not assumed; only the
 FBGraphObject protocol is assumed, like so:
 <pre>
 @textblock
   id<FBGraphObject> anyGraphObj;
 @/textblock
 </pre>
 As such, the methods declared on the FBGraphObject protocol represent the methods used by the SDK to 
 consume graph objects. While the FBGraphObject class implements the full NSMutableDictionary and KVC
 interfaces, these are not consumed directly by the SDK, and are optional for custom implementations.
 */
@protocol FBGraphObject<NSObject>

- (NSUInteger)count;
- (id)objectForKey:(id)aKey;
- (NSEnumerator *)keyEnumerator;
- (void)removeObjectForKey:(id)aKey;
- (void)setObject:(id)anObject forKey:(id)aKey;

@end

/*!
 @class
 
 @abstract
 Static class with helpers for use with graph objects
 
 @discussion
 The public interface of this class is useful for creating objects that have the same graph characteristics
 of those returned by methods of the SDK. This class also represents the internal implementation of the
 FBGraphObject protocol, used by the Facebook SDK. Application code should not use the FBGraphObject class to 
 access instances and instance members, favoring the protocol.
 */
@interface FBGraphObject : NSMutableDictionary<FBGraphObject>

/*!
 @method
 @abstract
 Used to create a graph object for, usually for use in posting a new graph object or action
 */
+ (NSMutableDictionary<FBGraphObject>*)graphObject;

/*!
 @method
 @abstract
 Used to wrap an existing dictionary with a FBGraphObject facade

 @discussion
 Normally you will not need to call this method, as the Facebook SDK already "FBGraphObject-ifys" json objects
 fetch via FBRequest/FBRequest connection. However, you may have other reasons to create json objects in your 
 application, which you would like to treat as a graph object. The pattern for doing this is that you pass the root
 node of the json to this method, to retrieve a wrapper. From this point, if you traverse the graph, any other objects
 deeper in the hierarchy will be wrapped as FBGraphObject's in a lazy fashion.
 
 This method is designed to avoid unnecessary memory allocations, and object copying. Due to this, the method does
 not copy the source object if it can be avoided, but rather wraps and uses it as is. The returned object derives 
 callers shoudl use the returned object after calls to this method, rather than continue to call methods on the original
 object.
 */
+ (NSMutableDictionary<FBGraphObject>*)graphObjectWrappingDictionary:(NSDictionary*)jsonDictionary;

/*!
 @method
 @abstract
 Used to compare two graph objects to determine if they are the same object.
 */
+ (BOOL)isGraphObjectID:(id<FBGraphObject>)anObject sameAs:(id<FBGraphObject>)anotherObject;


@end