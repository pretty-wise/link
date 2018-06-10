import { Component, OnInit } from '@angular/core';
import { Observable } from 'rxjs';
import { PluginService } from '../plugin.service';
import { Plugin } from '../shared/plugin';

@Component({
  selector: 'app-monitor',
  templateUrl: './monitor.component.html',
  styleUrls: ['./monitor.component.css']
})
export class MonitorComponent implements OnInit {

  constructor(
    private pluginService: PluginService) { }

  monitors: Observable<Plugin[]>;

  ngOnInit() {
    this.monitors = this.pluginService.getMonitorList();
  }

}
