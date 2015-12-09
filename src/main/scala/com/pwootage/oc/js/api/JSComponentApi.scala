package com.pwootage.oc.js.api

import java.nio.charset.StandardCharsets
import java.util

import com.pwootage.oc.js.AsyncMethodCaller
import li.cil.oc.api.machine.{LimitReachedException, Machine}
import li.cil.oc.api.network.Component

import scala.collection.JavaConversions._
import scala.collection.JavaConverters._
import scala.concurrent._
import scala.concurrent.duration._
import scala.math.ScalaNumber

class JSComponentApi(machine: Machine, sync: AsyncMethodCaller) {
  def list(name: String): util.List[util.Map[String, String]] = machine.components.synchronized {
    machine.components().filter(t => t._2.contains(name)).map(t => Map(
      "uuid" -> t._1,
      "type" -> t._2
    ).asJava).toList.asJava
  }

  def invoke(address: String, method: String, args: Array[AnyRef]): Array[AnyRef] = withComponent(address) { comp =>
    val m = machine.methods(comp.host).get(method)
    val invokeResult = if (m == null) null
    else {
      var res: Option[Array[AnyRef]] = None
      if (m.direct()) {
        try res = Some(machine.invoke(address, method, args)) catch {
          case e: LimitReachedException => //Ignore and call sync
          case e: Throwable => throw e
        }
      }
      res match {
        case Some(x) => x
        case None =>
          //Sync call
          Await.result(sync.callSync(() => machine.invoke(address, method, args)), 10 seconds)
      }
    }
    scalaToJS(invokeResult)
  }

  def scalaToJS(invokeResult: Array[AnyRef]): Array[AnyRef] = {
    def conv(v: AnyRef) = v match {
      case a: Array[Byte] => new Predef.String(a, StandardCharsets.UTF_8)
      case v: ScalaNumber => Array(v.underlying())
      case x => x
    }
    invokeResult.map(conv)
  }

  def doc(address: String, method: String): String = withComponent(address) { comp =>
    Option(machine.methods(comp.host).get(method)).map(_.doc).orNull
  }

  def methods(address: String): util.Map[String, util.Map[String, Any]] = withComponent(address) { comp =>
    machine.methods(comp.host).map(t => {
      val (name, callback) = t
      (name, Map[String, Any](
        "name" -> name,
        "doc" -> callback.doc(),
        "direct" -> callback.direct(),
        "limit" -> callback.limit(),
        "getter" -> callback.getter(),
        "setter" -> callback.setter()
      ).asJava)
    }).toMap.asJava
  }

  def `type`(address: String): String = machine.components.synchronized {
    machine.components().get(address)
  }

  private def withComponent[T](address: String)(f: (Component) => T): T = Option(machine.node.network.node(address)) match {
    case Some(component: Component) if component.canBeSeenFrom(machine.node) || component == machine.node =>
      f(component)
    case _ =>
      null.asInstanceOf[T] //eew
  }
}
